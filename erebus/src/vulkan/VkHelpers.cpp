#include "vulkan/vulkan.hpp"
#include <erebus/vulkan/VkHelpers.h>

#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

#include <cstdint>

namespace erb
{
	std::vector<const char*> getRequiredInstanceExtensions(bool isValidationLayersEnabled)
	{
		std::uint32_t glfwExtensionCount = 0;
		auto*         glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (isValidationLayersEnabled)
			extensions.push_back(vk::EXTDebugUtilsExtensionName);

		return extensions;
	}

	std::uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities)
	{
		auto minImageCount = std::max(3U, surfaceCapabilities.minImageCount);

		if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
			minImageCount = surfaceCapabilities.maxImageCount;

		return minImageCount;
	}

	vk::raii::ImageView createImageView(vk::raii::Device& device, const vk::Image& image, vk::Format format,
	    vk::ImageAspectFlags aspectFlags, std::uint32_t mipLevels)
	{
		vk::ImageViewCreateInfo viewInfo{
		    .image            = image,
		    .viewType         = vk::ImageViewType::e2D,
		    .format           = format,
		    .subresourceRange = {
		        .aspectMask     = aspectFlags,
		        .baseMipLevel   = 0,
		        .levelCount     = mipLevels,
		        .baseArrayLayer = 0,
		        .layerCount     = 1}};

		return {device, viewInfo};
	}

	std::uint32_t findMemoryType(vk::raii::PhysicalDevice& device, std::uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();
		for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) != 0U && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
	    vk::MemoryPropertyFlags properties)
	{
		vk::BufferCreateInfo bufferInfo{
		    .size        = size,
		    .usage       = usage,
		    .sharingMode = vk::SharingMode::eExclusive};

		vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferInfo);

		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{
		    .allocationSize  = memRequirements.size,
		    .memoryTypeIndex = findMemoryType(physDevice, memRequirements.memoryTypeBits, properties)};

		vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);

		return {std::move(buffer), std::move(bufferMemory)};
	}

	vk::SampleCountFlagBits getMaxUsableSampleCount(vk::raii::PhysicalDevice& device)
	{
		vk::PhysicalDeviceProperties physicalDeviceProperties = device.getProperties();

		vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & vk::SampleCountFlagBits::e64)
			return vk::SampleCountFlagBits::e64;

		if (counts & vk::SampleCountFlagBits::e32)
			return vk::SampleCountFlagBits::e32;

		if (counts & vk::SampleCountFlagBits::e16)
			return vk::SampleCountFlagBits::e16;

		if (counts & vk::SampleCountFlagBits::e8)
			return vk::SampleCountFlagBits::e8;

		if (counts & vk::SampleCountFlagBits::e4)
			return vk::SampleCountFlagBits::e4;

		if (counts & vk::SampleCountFlagBits::e2)
			return vk::SampleCountFlagBits::e2;

		return vk::SampleCountFlagBits::e1;
	}

	vk::Format findSupportedFormat(vk::raii::PhysicalDevice& device, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
	{
		for (const auto& format : candidates)
		{
			vk::FormatProperties props = device.getFormatProperties(format);
			if (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
			    ((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)))
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format");
	}

	vk::Format findDepthFormat(vk::raii::PhysicalDevice& device)
	{
		return findSupportedFormat(device,
		    {vk::Format::eD32Sfloat,
		        vk::Format::eD32SfloatS8Uint,
		        vk::Format::eD24UnormS8Uint},
		    vk::ImageTiling::eOptimal,
		    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, std::uint32_t width,
	    std::uint32_t height, std::uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
	    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		vk::ImageCreateInfo imageInfo{
		    .imageType = vk::ImageType::e2D,
		    .format    = format,
		    .extent    = {
		        .width  = width,
		        .height = height,
		        .depth  = 1},
		    .mipLevels   = mipLevels,
		    .arrayLayers = 1,
		    .samples     = numSamples,
		    .tiling      = tiling,
		    .usage       = usage,
		    .sharingMode = vk::SharingMode::eExclusive};

		vk::raii::Image image = vk::raii::Image(device, imageInfo);

		vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{
		    .allocationSize  = memRequirements.size,
		    .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)};

		vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device, allocInfo);
		image.bindMemory(imageMemory, 0);

		return {std::move(image), std::move(imageMemory)};
	}

	vk::raii::CommandBuffer beginSingleTimeCommands(vk::raii::Device& device, vk::raii::CommandPool& commandPool)
	{
		vk::CommandBufferAllocateInfo allocInfo{
		    .commandPool        = commandPool,
		    .level              = vk::CommandBufferLevel::ePrimary,
		    .commandBufferCount = 1};

		vk::raii::CommandBuffer    commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
		vk::CommandBufferBeginInfo beginInfo{
		    .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

		commandBuffer.begin(beginInfo);
		return std::move(commandBuffer);
	}

	void endSingleTimeCommand(vk::raii::CommandBuffer commandBuffer, vk::raii::Queue& targetQueue)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo{
		    .commandBufferCount = 1,
		    .pCommandBuffers    = &*commandBuffer};

		targetQueue.submit(submitInfo, nullptr);
		targetQueue.waitIdle();
	}

	void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image, vk::ImageLayout oldLayout,
	    vk::ImageLayout newLayout, std::uint32_t mipLevels)
	{
		vk::ImageMemoryBarrier barrier{
		    .oldLayout           = oldLayout,
		    .newLayout           = newLayout,
		    .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		    .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		    .image               = image,
		    .subresourceRange    = {
		        .aspectMask = vk::ImageAspectFlagBits::eColor,
		        .levelCount = mipLevels,
		        .layerCount = 1}};

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage      = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::runtime_error("unsupported layout transition");
		}

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
	}

	void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	    vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
	    vk::ImageAspectFlags imageAspectFlags)
	{
		vk::ImageMemoryBarrier2 barrier = {
		    .srcStageMask        = srcStageMask,
		    .srcAccessMask       = srcAccessMask,
		    .dstStageMask        = dstStageMask,
		    .dstAccessMask       = dstAccessMask,
		    .oldLayout           = oldLayout,
		    .newLayout           = newLayout,
		    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .image               = image,
		    .subresourceRange    = {
		        .aspectMask     = imageAspectFlags,
		        .baseMipLevel   = 0,
		        .levelCount     = 1,
		        .baseArrayLayer = 0,
		        .layerCount     = 1}};

		vk::DependencyInfo dependencyInfo{
		    .dependencyFlags         = {},
		    .imageMemoryBarrierCount = 1,
		    .pImageMemoryBarriers    = &barrier};

		commandBuffer.pipelineBarrier2(dependencyInfo);
	}

	void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::raii::Image& image,
	    std::uint32_t width, std::uint32_t height)
	{
		vk::BufferImageCopy region{
		    .bufferOffset      = 0,
		    .bufferRowLength   = 0,
		    .bufferImageHeight = 0,
		    .imageSubresource  = {
		        .aspectMask     = vk::ImageAspectFlagBits::eColor,
		        .mipLevel       = 0,
		        .baseArrayLayer = 0,
		        .layerCount     = 1},
		    .imageOffset = {.x = 0, .y = 0, .z = 0},
		    .imageExtent = {.width = width, .height = height, .depth = 1}};

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	}

	void generateMipMaps(vk::raii::PhysicalDevice& physicalDevice, vk::raii::CommandBuffer& commandBuffer,
	    vk::raii::Image& image, vk::Format imageFormat, std::int32_t texWidth, std::int32_t texHeight, std::uint32_t mipLevels)
	{
		vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);
		if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
			throw std::runtime_error("texture image format does not support linear blitting!");

		vk::ImageMemoryBarrier barrier = {
		    .srcAccessMask       = vk::AccessFlagBits::eTransferWrite,
		    .dstAccessMask       = vk::AccessFlagBits::eTransferRead,
		    .oldLayout           = vk::ImageLayout::eTransferDstOptimal,
		    .newLayout           = vk::ImageLayout::eTransferSrcOptimal,
		    .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		    .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		    .image               = image,
		    .subresourceRange    = {
		        .aspectMask = vk::ImageAspectFlagBits::eColor,
		        .levelCount = 1,
		        .layerCount = 1}};

		std::int32_t mipWidth  = texWidth;
		std::int32_t mipHeight = texHeight;

		for (std::uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;

			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;

			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

			vk::ImageBlit blit = {
			    .srcSubresource = {
			        .aspectMask = vk::ImageAspectFlagBits::eColor,
			        .mipLevel   = i - 1,
			        .layerCount = 1},
			    .srcOffsets     = std::array<vk::Offset3D, 2>{{{}, {.x = mipWidth, .y = mipHeight, .z = 1}}},
			    .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = i, .layerCount = 1},
			    .dstOffsets     = std::array<vk::Offset3D, 2>{{{}, {.x = 1 < mipWidth ? mipWidth / 2 : 1, .y = 1 < mipHeight ? mipHeight / 2 : 1, .z = 1}}}};

			commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image,
			    vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

			barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
			    vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

			if (1 < mipWidth)
				mipWidth /= 2;

			if (1 < mipHeight)
				mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;

		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
		    vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
	}

	void copyBuffer(vk::raii::Device& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue,
	    vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
	{
		vk::raii::CommandBuffer commandBufferCopy = beginSingleTimeCommands(device, commandPool);
		commandBufferCopy.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
		endSingleTimeCommand(std::move(commandBufferCopy), targetQueue);
	}
} // namespace erb
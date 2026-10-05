#pragma once

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <vector>
#include <cstdint>
#include <utility>

namespace erb
{
	std::vector<const char*> getRequiredInstanceExtensions(bool isValidationLayersEnabled = false);

	std::uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);

	vk::raii::ImageView createImageView(vk::raii::Device& device, const vk::Image& image, vk::Format format,
	    vk::ImageAspectFlags aspectFlags, std::uint32_t mipLevels);

	std::uint32_t findMemoryType(vk::raii::PhysicalDevice& device, std::uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
	    vk::MemoryPropertyFlags properties);

	vk::SampleCountFlagBits getMaxUsableSampleCount(vk::raii::PhysicalDevice& device);

	vk::Format findSupportedFormat(vk::raii::PhysicalDevice& device, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format findDepthFormat(vk::raii::PhysicalDevice& device);

	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, std::uint32_t width,
	    std::uint32_t height, std::uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
	    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

	vk::raii::CommandBuffer beginSingleTimeCommands(vk::raii::Device& device, vk::raii::CommandPool& commandPool);

	void endSingleTimeCommand(vk::raii::CommandBuffer commandBuffer, vk::raii::Queue& targetQueue);

	void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Image& image, vk::ImageLayout oldLayout,
	    vk::ImageLayout newLayout, std::uint32_t mipLevels);

	void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	    vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
	    vk::ImageAspectFlags imageAspectFlags);

	void copyBufferToImage(vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, vk::raii::Image& image,
	    std::uint32_t width, std::uint32_t height);

	void generateMipMaps(vk::raii::PhysicalDevice& physicalDevice, vk::raii::CommandBuffer& commandBuffer,
	    vk::raii::Image& image, vk::Format imageFormat, std::int32_t texWidth, std::int32_t texHeight, std::uint32_t mipLevels);

	void copyBuffer(vk::raii::Device& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue,
	    vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
} // namespace erb
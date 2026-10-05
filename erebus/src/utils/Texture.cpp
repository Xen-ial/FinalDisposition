#include "vulkan/vulkan.hpp"
#include <erebus/utils/Texture.h>
#include <erebus/vulkan/VkHelpers.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ktx.h>

#include <print>

namespace erb
{
	bool Texture::createFromFile(const std::string& path, bool useMipMaps)
	{
		int texChannels{};
		int width{};
		int height{};

		stbi_uc*       pixels    = stbi_load(path.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(width) * height * 4;

		if (useMipMaps)
			m_mipLevels = static_cast<std::uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		if (pixels == nullptr)
		{
			std::println("failed to load texture image: {}", path);
			return false;
		}

		m_texWidth  = width;
		m_texHeight = height;

		auto& device         = m_device.get();
		auto& physicalDevice = device.getPhysicalDevice();

		auto [stagingBuffer, stagingBufferMemory] = createBuffer(device.getDevice(), physicalDevice, imageSize,
		    vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void* data = stagingBufferMemory.mapMemory(0, imageSize);
		memcpy(data, pixels, imageSize);
		stagingBufferMemory.unmapMemory();

		vk::Format textureFormat = vk::Format::eR8G8B8A8Srgb;

		vk::SampleCountFlagBits mipMaps = useMipMaps ? getMaxUsableSampleCount(physicalDevice) : vk::SampleCountFlagBits::e1;

		std::tie(m_textureImage, m_textureImageMemory) =
		    createImage(
		        device.getDevice(), physicalDevice,
		        m_texWidth, m_texHeight,
		        m_mipLevels, mipMaps,
		        textureFormat, vk::ImageTiling::eOptimal,
		        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		        vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(device.getDevice(), m_commandPool.get());
		transitionImageLayout(commandBuffer, m_textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_mipLevels);
		copyBufferToImage(commandBuffer, stagingBuffer, m_textureImage, static_cast<std::uint32_t>(m_texWidth), static_cast<std::uint32_t>(m_texHeight));

		generateMipMaps(physicalDevice, commandBuffer, m_textureImage, textureFormat, width, height, m_mipLevels);

		endSingleTimeCommand(std::move(commandBuffer), m_targetQueue.get());
		stbi_image_free(pixels);

		m_textureImageView = createImageView(device.getDevice(), *m_textureImage,
		    vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, m_mipLevels);

		return createTextureImageSampler();
	}

	bool Texture::ktxCreateFromFile(const std::string& path, bool useMipMaps)
	{
		ktxTexture*    kTexture = nullptr;
		KTX_error_code result =
		    ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);

		if (result != KTX_SUCCESS)
		{
			std::println("failed to create texture: {}", path);
			return false;
		}

		std::uint32_t texWdith  = kTexture->baseWidth;
		std::uint32_t texHeight = kTexture->baseHeight;

		m_texWidth  = texWdith;
		m_texHeight = texHeight;

		ktx_size_t   imageSize      = ktxTexture_GetImageSize(kTexture, 0);
		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(kTexture);

		if (useMipMaps)
			m_mipLevels = static_cast<std::uint32_t>(std::floor(std::log2(std::max(texWdith, texHeight)))) + 1;

		auto& device                              = m_device.get();
		auto [stagingBuffer, stagingBufferMemory] = createBuffer(device.getDevice(), device.getPhysicalDevice(),
		    imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void* data = stagingBufferMemory.mapMemory(0, imageSize);
		memcpy(data, ktxTextureData, imageSize);
		stagingBufferMemory.unmapMemory();

		vk::Format textureFormat = vk::Format::eR8G8B8A8Srgb;

		vk::SampleCountFlagBits mipMaps = useMipMaps ? getMaxUsableSampleCount(device.getPhysicalDevice()) : vk::SampleCountFlagBits::e1;

		std::tie(m_textureImage, m_textureImageMemory) = createImage(
		    device.getDevice(), device.getPhysicalDevice(),
		    texWdith, texHeight,
		    m_mipLevels, mipMaps,
		    textureFormat, vk::ImageTiling::eOptimal,
		    vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		    vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(device.getDevice(), m_commandPool.get());
		transitionImageLayout(commandBuffer, m_textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_mipLevels);
		copyBufferToImage(commandBuffer, stagingBuffer, m_textureImage, texWdith, texHeight);

		generateMipMaps(device.getPhysicalDevice(), commandBuffer, m_textureImage,
		    textureFormat, static_cast<std::int32_t>(texWdith), static_cast<std::int32_t>(texHeight), m_mipLevels);

		endSingleTimeCommand(std::move(commandBuffer), m_targetQueue.get());
		ktxTexture_Destroy(kTexture);

		m_textureImageView = createImageView(device.getDevice(), *m_textureImage,
		    vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, m_mipLevels);

		return createTextureImageSampler();
	}

	bool Texture::createTextureImageSampler()
	{
		vk::PhysicalDeviceProperties props = m_device.get().getPhysicalDevice().getProperties();

		vk::SamplerCreateInfo samplerInfo{
		    .magFilter        = vk::Filter::eLinear,
		    .minFilter        = vk::Filter::eLinear,
		    .mipmapMode       = vk::SamplerMipmapMode::eLinear,
		    .addressModeU     = vk::SamplerAddressMode::eRepeat,
		    .addressModeV     = vk::SamplerAddressMode::eRepeat,
		    .addressModeW     = vk::SamplerAddressMode::eRepeat,
		    .mipLodBias       = 0.0F,
		    .anisotropyEnable = vk::True,
		    .maxAnisotropy    = props.limits.maxSamplerAnisotropy,
		    .compareEnable    = vk::False,
		    .compareOp        = vk::CompareOp::eAlways,
		    .minLod           = 0.0F,
		    .maxLod           = vk::LodClampNone};

		m_textureImageSampler = vk::raii::Sampler(m_device.get().getDevice(), samplerInfo);
		return true;
	}
} // namespace erb
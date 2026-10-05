#pragma once
#include <erebus/vulkan/VkDevice.h>

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <string>
#include <functional>

namespace erb
{
	class Texture
	{
	public:
		Texture(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue)
		    : m_device(device), m_commandPool(commandPool), m_targetQueue(targetQueue) {}

		bool createFromFile(const std::string& path, bool useMipMaps = false);
		bool ktxCreateFromFile(const std::string& path, bool useMipMaps = false);

		vk::raii::Image&        getTextureImage() { return m_textureImage; }
		vk::raii::DeviceMemory& getTextureImageMemory() { return m_textureImageMemory; }
		vk::raii::ImageView&    getTextureImageView() { return m_textureImageView; }
		vk::raii::Sampler&      getTextureImageSampler() { return m_textureImageSampler; }

		vk::raii::CommandPool& getCommandPool() { return m_commandPool; }
		vk::raii::Queue&       getQueue() { return m_targetQueue; }

		[[nodiscard]] std::uint32_t getWidth() const { return m_texWidth; }
		[[nodiscard]] std::uint32_t getHeight() const { return m_texHeight; }

	private:
		std::reference_wrapper<VkDevice>              m_device;
		std::reference_wrapper<vk::raii::CommandPool> m_commandPool;
		std::reference_wrapper<vk::raii::Queue>       m_targetQueue;

		std::uint32_t m_mipLevels{1};
		std::uint32_t m_texWidth{};
		std::uint32_t m_texHeight{};

		vk::raii::Image        m_textureImage       = nullptr;
		vk::raii::DeviceMemory m_textureImageMemory = nullptr;
		vk::raii::ImageView    m_textureImageView   = nullptr;

		vk::raii::Sampler m_textureImageSampler = nullptr;

		bool createTextureImageSampler();
	};
} // namespace erb
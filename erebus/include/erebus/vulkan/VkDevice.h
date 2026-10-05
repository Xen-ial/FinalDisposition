#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <optional>
#include <vector>
#include <functional>

namespace erb
{
	struct QueueFamilyIndices
	{
		std::optional<std::uint32_t> graphicsFamily;
		std::optional<std::uint32_t> presentFamily;
		std::optional<std::uint32_t> computeFamily;

		[[nodiscard]] bool isComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;

		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR>   presentModes;
	};

	class VkDevice
	{
	public:
		VkDevice(const VkDevice&)            = delete;
		VkDevice& operator=(const VkDevice&) = delete;

		VkDevice(VkDevice&&)            = delete;
		VkDevice& operator=(VkDevice&&) = delete;

		VkDevice(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface,
		    const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions);

		~VkDevice() = default;

		bool pickPhysicalDevice();
		bool createLogicalDevice(bool enableValidationLayers, const std::vector<const char*>& validationLayers);

		QueueFamilyIndices      findQueueFamilies(vk::raii::PhysicalDevice& physicalDevice);
		SwapChainSupportDetails querySwapChainSupport(vk::raii::PhysicalDevice& physicalDevice);

		[[nodiscard]] std::uint32_t findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

		vk::raii::PhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
		vk::raii::Device&         getDevice() { return m_device; }

		vk::raii::Queue& getGraphicsQueue() { return m_graphicsQueue; }
		vk::raii::Queue& getComputeQueue() { return m_computeQueue; }
		vk::raii::Queue& getPresentationQueue() { return m_presentQueue; }

		[[nodiscard]] QueueFamilyIndices getQueueFamiltIndices() const { return m_queueFamilyIndices; }

		vk::raii::SurfaceKHR& getSurfaceKHR() { return m_surface; }

	private:
		std::reference_wrapper<vk::raii::Instance>   m_instance;
		std::reference_wrapper<vk::raii::SurfaceKHR> m_surface;

		vk::raii::PhysicalDevice m_physicalDevice = nullptr;
		vk::raii::Device         m_device         = nullptr;

		vk::raii::Queue m_graphicsQueue = nullptr;
		vk::raii::Queue m_presentQueue  = nullptr;
		vk::raii::Queue m_computeQueue  = nullptr;

		QueueFamilyIndices m_queueFamilyIndices{};

		std::vector<const char*> m_requiredExtensions;
		std::vector<const char*> m_optionalExtensions;
		std::vector<const char*> m_deviceExtensions;

		bool isDeviceSuitable(vk::raii::PhysicalDevice& physicalDevice);
		bool checkDeviceExtensionSupport(vk::raii::PhysicalDevice& physicalDevice);

		static bool supportsAllFeatures(vk::raii::PhysicalDevice& physicalDevice);
	};
} // namespace erb
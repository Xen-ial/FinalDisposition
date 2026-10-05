#include <erebus/vulkan/VkDevice.h>

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <string>
#include <set>
#include <print>
#include <map>
#include <exception>

namespace erb
{
	VkDevice::VkDevice(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface,
	    const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions)
	    : m_instance(instance), m_surface(surface),
	      m_requiredExtensions(requiredExtensions), m_optionalExtensions(optionalExtensions), m_deviceExtensions(requiredExtensions)
	{
		m_deviceExtensions.insert(m_deviceExtensions.end(), optionalExtensions.begin(), optionalExtensions.end());
	}

	bool VkDevice::pickPhysicalDevice()
	{
		try
		{
			std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.get().enumeratePhysicalDevices();
			if (physicalDevices.empty())
			{
				std::println("query failed to find any GPU that supports vulkan");
				return false;
			}

			std::multimap<int, vk::raii::PhysicalDevice> suitableDevices;

			for (auto& device : physicalDevices)
			{
				vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
				std::println("checking device: {}", std::string(deviceProperties.deviceName));

				bool supportsVulkan13 = deviceProperties.apiVersion >= vk::ApiVersion13;
				if (!supportsVulkan13)
				{
					std::println("device does not support vulkan 1.3");
					continue;
				}

				QueueFamilyIndices indices          = findQueueFamilies(device);
				bool               supportsGraphics = indices.isComplete();
				if (!supportsGraphics)
				{
					std::println("device does not support graphics");
					continue;
				}

				bool supportsAllRequiredExtensions = checkDeviceExtensionSupport(device);
				if (!supportsAllRequiredExtensions)
				{
					std::println("missing required extensions");
					continue;
				}

				bool swapChainAdequate = false;
				if (supportsAllRequiredExtensions)
				{
					SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
					swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

					if (!swapChainAdequate)
					{
						std::println("inadequate swap chain support");
						continue;
					}
				}

				bool supportsRequiredFeatures = supportsAllFeatures(device);
				if (!supportsRequiredFeatures)
				{
					std::println("does not support required features");
					continue;
				}

				int score{};

				const int discreteGpuScore   = 1000;
				const int integratedGouScore = 100;
				const int gigabytes          = 1024 * 1024 * 1024;

				if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				{
					score += discreteGpuScore;
					std::println("discrete gpu +1000 points");
				}

				if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
				{
					score += integratedGouScore;
					std::println("integrated gpu +100 points");
				}

				vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();
				for (std::uint32_t i = 0; i < memProperties.memoryHeapCount; i++)
				{
					const auto& memoryHeap = memProperties.memoryHeaps[i];

					if (memoryHeap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
					{
						score += static_cast<int>(memoryHeap.size) / gigabytes;
						break;
					}
				}

				std::println("device sore: {}", score);
				suitableDevices.emplace(score, device);
			}

			if (!suitableDevices.empty())
			{
				m_physicalDevice = suitableDevices.rbegin()->second;

				vk::PhysicalDeviceProperties deviceProperties = m_physicalDevice.getProperties();
				std::println("selected device: {}", std::string(deviceProperties.deviceName));

				m_queueFamilyIndices = findQueueFamilies(m_physicalDevice);
				return true;
			}

			std::println("failed to find suitable gpu");
			return false;
		}
		catch (const std::exception& excpt)
		{
			std::println("failed to pick physical device: {}", excpt.what());
			return false;
		}
	}

	bool VkDevice::createLogicalDevice(bool enableValidationLayers, const std::vector<const char*>& validationLayers)
	{
		try
		{
			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::set<std::uint32_t>                uniqueQueueFamilies = {
			    m_queueFamilyIndices.graphicsFamily.value(),
			    m_queueFamilyIndices.presentFamily.value(),
			    m_queueFamilyIndices.computeFamily.value()};

			float queuePriority = 1.0F;
			for (std::uint32_t queueFamily : uniqueQueueFamilies)
			{
				vk::DeviceQueueCreateInfo queueCreateInfo{
				    .queueFamilyIndex = queueFamily,
				    .queueCount       = 1,
				    .pQueuePriorities = &queuePriority};

				queueCreateInfos.push_back(queueCreateInfo);
			}

			vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2{
			    .features = {
			        .samplerAnisotropy = vk::True},
			};

			vk::PhysicalDeviceVulkan11Features physicalDeviceVulkan11Features{
			    .shaderDrawParameters = vk::True};

			vk::PhysicalDeviceVulkan13Features physicalDeviceVulkan13Features{
			    .synchronization2 = vk::True,
			    .dynamicRendering = vk::True};

			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT physicalDeviceExtendedDynamicStateFeatures{
			    .extendedDynamicState = vk::True};

			vk::StructureChain<
			    vk::PhysicalDeviceFeatures2,
			    vk::PhysicalDeviceVulkan11Features,
			    vk::PhysicalDeviceVulkan13Features,
			    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
			    featureChain{
			        physicalDeviceFeatures2,
			        physicalDeviceVulkan11Features,
			        physicalDeviceVulkan13Features,
			        physicalDeviceExtendedDynamicStateFeatures};

			auto&                features = featureChain.get<vk::PhysicalDeviceFeatures2>();
			vk::DeviceCreateInfo createInfo{
			    .pNext                   = &features,
			    .queueCreateInfoCount    = static_cast<std::uint32_t>(queueCreateInfos.size()),
			    .pQueueCreateInfos       = queueCreateInfos.data(),
			    .enabledExtensionCount   = static_cast<std::uint32_t>(m_deviceExtensions.size()),
			    .ppEnabledExtensionNames = m_deviceExtensions.data(),
			    .pEnabledFeatures        = nullptr};

			m_device = vk::raii::Device(m_physicalDevice, createInfo);

			m_graphicsQueue = vk::raii::Queue(m_device, m_queueFamilyIndices.graphicsFamily.value(), 0);
			m_presentQueue  = vk::raii::Queue(m_device, m_queueFamilyIndices.presentFamily.value(), 0);
			m_computeQueue  = vk::raii::Queue(m_device, m_queueFamilyIndices.computeFamily.value(), 0);

			return true;
		}
		catch (const std::exception& exc)
		{
			std::println("failed to create logical device: {}", exc.what());
			return false;
		}
	}

	QueueFamilyIndices VkDevice::findQueueFamilies(vk::raii::PhysicalDevice& physicalDevice)
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

		for (std::uint32_t i = 0; i < queueFamilies.size(); i++)
		{
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
				indices.graphicsFamily = i;

			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
				indices.computeFamily = i;

			if (physicalDevice.getSurfaceSupportKHR(i, m_surface.get()) == vk::True)
				indices.presentFamily = i;

			if (indices.isComplete())
				break;
		}

		return indices;
	}

	SwapChainSupportDetails VkDevice::querySwapChainSupport(vk::raii::PhysicalDevice& physicalDevice)
	{
		SwapChainSupportDetails details;

		details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*m_surface.get());
		details.formats      = physicalDevice.getSurfaceFormatsKHR(*m_surface.get());
		details.presentModes = physicalDevice.getSurfacePresentModesKHR(*m_surface.get());

		return details;
	}

	bool VkDevice::checkDeviceExtensionSupport(vk::raii::PhysicalDevice& physicalDevice)
	{
		std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
		std::set<std::string>                requiredExtensionSet(m_requiredExtensions.begin(), m_requiredExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensionSet.erase(extension.extensionName);

		if (!requiredExtensionSet.empty())
		{
			std::println("missing required extensions.");

			for (const auto& extension : requiredExtensionSet)
				std::println("Extension: {}", std::string(extension));

			return false;
		}

		return true;
	}

	bool VkDevice::isDeviceSuitable(vk::raii::PhysicalDevice& physicalDevice)
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
		bool swapChainAdequate   = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
			swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		bool supportsRequiredFeatures = supportsAllFeatures(physicalDevice);
		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportsRequiredFeatures;
	}

	std::uint32_t VkDevice::findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
	{
		vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

		for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (((typeFilter & (1 << i)) != 0U) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw std::runtime_error("failed to find a suitable memory type.");
	}

	bool VkDevice::supportsAllFeatures(vk::raii::PhysicalDevice& physicalDevice)
	{
		auto features = physicalDevice.template getFeatures2<
		    vk::PhysicalDeviceFeatures2,
		    vk::PhysicalDeviceVulkan11Features,
		    vk::PhysicalDeviceVulkan13Features,
		    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

		bool supportsSamplerAnisotropy    = (features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy == vk::True);
		bool supporsShaderDrawParameters  = (features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters == vk::True);
		bool supportsDynamicRendering     = (features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering == vk::True);
		bool supportsExtendedDynamicState = (features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState == vk::True);

		bool supportsRequiredFeatures =
		    supportsSamplerAnisotropy &&
		    supporsShaderDrawParameters &&
		    supportsDynamicRendering &&
		    supportsExtendedDynamicState;

		return supportsRequiredFeatures;
	}
} // namespace erb
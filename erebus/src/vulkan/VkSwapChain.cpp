#include <erebus/vulkan/VkSwapChain.h>
#include <erebus/vulkan/VkHelpers.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <limits>
#include <print>

namespace erb
{
	VkSwapChain::VkSwapChain(VkDevice& device, Window& window)
	    : m_device(device), m_window(window)
	{
	}

	bool VkSwapChain::create()
	{
		vk::raii::PhysicalDevice& physicalDevice = m_device.get().getPhysicalDevice();

		auto swapChainSupport = m_device.get().querySwapChainSupport(physicalDevice);

		auto surfaceCapabilities = m_device.get().querySwapChainSupport(physicalDevice).capabilities;
		m_swapChainExtent        = chooseSwapExtent(surfaceCapabilities);

		std::uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

		std::vector<vk::SurfaceFormatKHR> availableFormats = swapChainSupport.formats;
		m_swapChainSurfaceFormat                           = chooseSwapSurfaceFormat(availableFormats);

		std::vector<vk::PresentModeKHR> availablePresentModes = swapChainSupport.presentModes;
		vk::PresentModeKHR              presentMode           = chooseSwapPresentMode(availablePresentModes);

		vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		    .surface          = *m_device.get().getSurfaceKHR(),
		    .minImageCount    = minImageCount,
		    .imageFormat      = m_swapChainSurfaceFormat.format,
		    .imageColorSpace  = m_swapChainSurfaceFormat.colorSpace,
		    .imageExtent      = m_swapChainExtent,
		    .imageArrayLayers = 1,
		    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
		    .imageSharingMode = vk::SharingMode::eExclusive,
		    .preTransform     = surfaceCapabilities.currentTransform,
		    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		    .presentMode      = presentMode,
		    .clipped          = vk::True,
		    .oldSwapchain     = nullptr};

		m_swapChain       = vk::raii::SwapchainKHR(m_device.get().getDevice(), swapChainCreateInfo);
		m_swapChainImages = m_swapChain.getImages();

		return true;
	}

	bool VkSwapChain::createImageViews()
	{
		assert(m_swapChainImageViews.empty());
		m_swapChainImageViews.reserve(m_swapChainImages.size());

		for (auto& image : m_swapChainImages)
			m_swapChainImageViews.emplace_back(createImageView(m_device.get().getDevice(), image, m_swapChainSurfaceFormat.format,
			    vk::ImageAspectFlagBits::eColor, 1));

		return true;
	}

	void VkSwapChain::cleanup()
	{
		m_swapChainImageViews.clear();
		m_swapChain = nullptr;
	}

	bool VkSwapChain::recreate()
	{
		int width{};
		int height{};

		auto& window = m_window.get();

		glfwGetFramebufferSize(window.handle, &width, &height);
		while (width == 0 || height == 0 || !window.shouldClose())
		{
			glfwGetFramebufferSize(window.handle, &width, &height);
			glfwWaitEvents();
		}

		if (window.shouldClose())
			return false;

		auto& device = m_device.get().getDevice();
		device.waitIdle();

		cleanup();

		if (!create())
		{
			std::println("failed to recreate swapchain");
			return false;
		}

		if (!createImageViews())
		{
			std::println("failed to recreate imageviews");
			return false;
		}

		return true;
	}

	vk::SurfaceFormatKHR VkSwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		const auto formatIt = std::ranges::find_if(availableFormats,
		    [](const vk::SurfaceFormatKHR& format)
		    {
			    return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		    });

		return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
	}

	vk::PresentModeKHR VkSwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
	{
		assert(std::ranges::any_of(availablePresentModes,
		    [](vk::PresentModeKHR presentMode)
		    {
			    return presentMode == vk::PresentModeKHR::eFifo;
		    }));

		return std::ranges::any_of(availablePresentModes,
		           [](const vk::PresentModeKHR value)
		           {
			           return vk::PresentModeKHR::eMailbox == value;
		           })
		    ? vk::PresentModeKHR::eMailbox
		    : vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D VkSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
			return capabilities.currentExtent;

		int width{};
		int height{};

		glfwGetFramebufferSize(m_window.get().handle, &width, &height);
		return {
		    .width  = std::clamp<std::uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		    .height = std::clamp<std::uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
	}
} // namespace erb
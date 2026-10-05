#pragma once
#include "vulkan/vulkan.hpp"
#include <erebus/vulkan/VkDevice.h>
#include <erebus/engine/Window.h>

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>

#include <functional>

namespace erb
{
	class VkSwapChain
	{
	public:
		VkSwapChain(const VkSwapChain&)            = delete;
		VkSwapChain& operator=(const VkSwapChain&) = delete;

		VkSwapChain(VkSwapChain&&)            = delete;
		VkSwapChain& operator=(VkSwapChain&&) = delete;

		VkSwapChain(VkDevice& device, Window& window);

		~VkSwapChain() = default;

		bool create();

		bool createImageViews();

		void cleanup();

		bool recreate();

		vk::raii::SwapchainKHR& getSwapChain() { return m_swapChain; }

		[[nodiscard]] const std::vector<vk::Image>& getSwapChainImages() const { return m_swapChainImages; }

		[[nodiscard]] vk::Format getSwapChainImageFormat() const { return m_swapChainImageFormat; }

		[[nodiscard]] vk::Extent2D getSwapChainExtent() const { return m_swapChainExtent; }

		vk::SurfaceFormatKHR& getSwapChainSurfaceFormat() { return m_swapChainSurfaceFormat; }

		[[nodiscard]] const std::vector<vk::raii::ImageView>& getSwapChainImageViews() const { return m_swapChainImageViews; }

	private:
		std::reference_wrapper<VkDevice> m_device;
		std::reference_wrapper<Window>   m_window;

		vk::raii::SwapchainKHR m_swapChain = nullptr;
		std::vector<vk::Image> m_swapChainImages;

		vk::Format                       m_swapChainImageFormat = vk::Format::eUndefined;
		std::vector<vk::raii::ImageView> m_swapChainImageViews;

		vk::Extent2D m_swapChainExtent = {.width = 0, .height = 0};

		vk::SurfaceFormatKHR m_swapChainSurfaceFormat;

		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		static vk::PresentModeKHR   chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
		vk::Extent2D                chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	};
} // namespace erb
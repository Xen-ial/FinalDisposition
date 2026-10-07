#pragma once
#include <erebus/engine/Window.h>
#include <erebus/engine/Camera.h>

#include <erebus/utils/ResourceManager.h>

#include <erebus/vulkan/VkDevice.h>
#include <erebus/vulkan/VkSwapChain.h>
#include <erebus/vulkan/VkShader.h>
#include <erebus/vulkan/VkDescriptorManager.h>
#include <erebus/vulkan/RenderObject.h>

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <cstdint>
#include <optional>
#include <functional>

namespace erb
{
	constexpr std::uint32_t RMAX_FRAMES_IN_FLIGHT = 2;

	struct Pipeline
	{
		vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout      pipelineLayout      = nullptr;
		vk::raii::Pipeline            pipeline            = nullptr;
	};

	class Renderer
	{
	public:
		Renderer(Window& window) : m_window(window) {}

		bool init(const std::string& applicationName);

		void setFramebufferSize(std::uint32_t width, std::uint32_t height);

		void addRenderObject(RenderObject* renderObject);
		void removeRenderObject(RenderObject* renderObject);

		static Model createModel(const std::string& path);
		Texture      createTexture(const std::string& path, bool useMipMaps = false);

		void render(Camera3D& camera);
		void cleanup();
		void destroyOldObjects();

		Window&              getWindow() { return m_window; }
		ResourceManager&     getResources() { return m_resourceManager; }
		VkDescriptorManager& getDescriptorManager() { return m_descriptorManager.value(); }

		Pipeline& getUnlitPipeline() { return m_unlitPipeline; }

		bool getHasInitialized() const { return m_hasInitialized; }

		std::unordered_map<std::uint32_t, RenderObject*>& getRenderObjects() { return m_renderObjects; }

		std::uint32_t getWidth() const { return m_width; }
		std::uint32_t getHeight() const { return m_height; }

		float getDeltaTime() const { return m_deltaTime; }

	private:
		std::string m_applicationName;

		float m_lastFrame{};
		float m_deltaTime{};

		std::uint32_t m_width{};
		std::uint32_t m_height{};

		std::uint32_t m_queueIndex = ~0;
		std::uint32_t m_frameIndex = 0;

		bool m_hasInitialized = false;

		std::reference_wrapper<Window> m_window;

		ResourceManager m_resourceManager{};

		vk::raii::Context  m_context;
		vk::raii::Instance m_instance = nullptr;

		vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;

		vk::raii::SurfaceKHR m_surface = nullptr;

		std::optional<VkDevice>    m_device;
		std::optional<VkSwapChain> m_swapChain;

		vk::SampleCountFlagBits m_msaaSamples{};

		Pipeline m_unlitPipeline;

		vk::raii::CommandPool                m_commandPool = nullptr;
		std::vector<vk::raii::CommandBuffer> m_commandBuffers;

		vk::raii::Image        m_colorImage       = nullptr;
		vk::raii::DeviceMemory m_colorImageMemory = nullptr;
		vk::raii::ImageView    m_colorImageView   = nullptr;

		vk::raii::Image        m_depthImage       = nullptr;
		vk::raii::DeviceMemory m_depthImageMemory = nullptr;
		vk::raii::ImageView    m_depthImageView   = nullptr;

		std::optional<VkDescriptorManager> m_descriptorManager;

		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphore;
		std::vector<vk::raii::Semaphore> m_presentCompletedSemaphore;
		std::vector<vk::raii::Fence>     m_inFlightFences;

		std::uint32_t                                    m_renderObjectId{};
		std::unordered_map<std::uint32_t, RenderObject*> m_renderObjects;

		bool createInstance();
		bool setupDebugMessenger();
		bool createSurface();
		bool createDevices();
		bool createSwapChain();
		bool createUnlitDescriptorSetLayout();
		bool createUnlitGraphicsPipeline();
		bool createCommandPool();
		bool createColorResources();
		bool createDepthResources();
		bool createCommandBuffers();
		bool createDescriptorManager();
		bool createSyncObjects();

		void onFramebufferResized();
		void recordCommandBuffer(std::uint32_t imageIndex);
		void updateUniformBuffers(Camera3D& camera);
		void updateFrameTime();
	};
} // namespace erb
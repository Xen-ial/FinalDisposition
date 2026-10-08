#include "erebus/vulkan/VkDescriptorManager.h"
#include "vulkan/vulkan.hpp"
#include <erebus/vulkan/Renderer.h>
#include <erebus/vulkan/VkHelpers.h>

#include <erebus/utils/ModelLoader.h>
#include <erebus/utils/Vertex.h>
#include <erebus/utils/Material.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <algorithm>
#include <print>
#include <functional>
#include <cstdint>

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

namespace
{
	VKAPI_ATTR vk::Bool32 VKAPI_CALL vulkDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	    vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		if (severity != vk::DebugUtilsMessageSeverityFlagBitsEXT::eError &&
		    severity != vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
			return vk::True;

		std::println("validation layer: {} msg: {}", to_string(type), pCallbackData->pMessage);
		return vk::False;
	}

} // namespace

namespace erb
{
#pragma region Initialization

	bool Renderer::init(const std::string& applicationName)
	{
		m_applicationName = applicationName;

		if (m_hasInitialized)
			return true;

		if (!createInstance())
		{
			std::println("initialization failed at createInstance");
			return false;
		}

		if (!setupDebugMessenger())
		{
			std::println("initialization failed at setupDebugMessenger");
			return false;
		}

		if (!createSurface())
		{
			std::println("initialization falied at createSurface");
			return false;
		}

		if (!createDevices())
		{
			std::println("initialization failed at createDevices");
			return false;
		}

		if (!createSwapChain())
		{
			std::println("initialization failed at createSwapChain");
			return false;
		}

		// if (!createUnlitDescriptorSetLayout())
		// {
		// 	std::println("initialization failed at createDescriptorLayout");
		// 	return false;
		// }

		// if (!createUnlitGraphicsPipeline())
		// {
		// 	std::println("initialization failed at createUnlitGraphicsPipeline");
		// 	return false;
		// }

		if (!createLightingDescriptorSetLayout())
		{
			std::println("initalization failed at createLightingDescriptorSetLayout");
			return false;
		}

		if (!createLightingGraphicsPipeline())
		{
			std::println("initalization failed at createLightingGraphicsPipeline");
			return false;
		}

		if (!createCommandPool())
		{
			std::println("initialization failed at createCommandPool");
			return false;
		}

		if (!createColorResources())
		{
			std::println("initalization failed at createColorResources");
			return false;
		}

		if (!createDepthResources())
		{
			std::println("initialization failed at createDepthResources");
			return false;
		}

		if (!createCommandBuffers())
		{
			std::println("initialization failed at createCommandBuffers");
			return false;
		}

		if (!createDescriptorManager())
		{
			std::println("initialization failed at createDescriptorManager");
			return false;
		}

		if (!createSyncObjects())
		{
			std::println("initialization failed at createSyncObjects");
			return false;
		}

		m_lastFrame      = static_cast<float>(glfwGetTime());
		m_hasInitialized = true;
		return true;
	}

	bool Renderer::createInstance()
	{
		if (m_hasInitialized)
			return true;

		vk::ApplicationInfo appInfo{
		    .pApplicationName   = m_applicationName.c_str(),
		    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		    .pEngineName        = "Erebus",
		    .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
		    .apiVersion         = vk::ApiVersion14};

		std::vector<const char*> requiredLayers;
		if (ENABLE_VALIDATION_LAYERS)
			requiredLayers.assign(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());

		std::vector<vk::LayerProperties> layerProperties = m_context.enumerateInstanceLayerProperties();

		auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
		    [&layerProperties](const auto& requiredLayer)
		    {
			    return std::ranges::none_of(layerProperties,
			        [requiredLayer](const vk::LayerProperties& layerProperty)
			        {
				        return strcmp(layerProperty.layerName, requiredLayer) == 0;
			        });
		    });

		if (unsupportedLayerIt != requiredLayers.end())
		{
			std::println("required layer is not supported: {}", std::string(*unsupportedLayerIt));
			m_hasInitialized = false;
			return false;
		}

		std::vector<const char*>             requiredExtensions  = getRequiredInstanceExtensions(ENABLE_VALIDATION_LAYERS);
		std::vector<vk::ExtensionProperties> extensionProperties = m_context.enumerateInstanceExtensionProperties();

		auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions,
		    [&extensionProperties](const auto& requiredExtension)
		    {
			    return std::ranges::none_of(extensionProperties,
			        [requiredExtension](const vk::ExtensionProperties& extensionProperty)
			        {
				        return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
			        });
		    });

		if (unsupportedPropertyIt != requiredExtensions.end())
		{
			std::println("required extension not supported: {}", std::string(*unsupportedPropertyIt));
			m_hasInitialized = false;
			return false;
		}

		std::uint32_t glfwExtensionCount = 0;
		auto*         glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		vk::InstanceCreateInfo createInfo{
		    .pApplicationInfo        = &appInfo,
		    .enabledLayerCount       = static_cast<std::uint32_t>(requiredLayers.size()),
		    .ppEnabledLayerNames     = requiredLayers.data(),
		    .enabledExtensionCount   = static_cast<std::uint32_t>(requiredExtensions.size()),
		    .ppEnabledExtensionNames = requiredExtensions.data()};

		m_instance = vk::raii::Instance(m_context, createInfo);
		return true;
	}

	bool Renderer::setupDebugMessenger()
	{
		if (m_hasInitialized)
			return true;

		if (!ENABLE_VALIDATION_LAYERS) return true;

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError};

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
		    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation};

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
		    .messageSeverity = severityFlags,
		    .messageType     = messageTypeFlags,
		    .pfnUserCallback = vulkDebugCallback};

		m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfo);
		return true;
	}

	bool Renderer::createSurface()
	{
		if (m_hasInitialized)
			return true;

		VkSurfaceKHR surface{};

		if (m_window.get().handle == nullptr)
		{
			std::println("failed to create glfw window surface, current target window is nullptr");
			m_hasInitialized = false;
			return false;
		}

		if (glfwCreateWindowSurface(*m_instance, m_window.get().handle, nullptr, &surface) != VK_FALSE)
		{
			std::println("failed to create glfw window surface");
			m_hasInitialized = false;
			return false;
		}

		m_surface = vk::raii::SurfaceKHR(m_instance, surface);
		return true;
	}

	bool Renderer::createDevices()
	{
		if (m_hasInitialized)
			return true;

		const std::vector<const char*> requiredExtensions = {
		    vk::KHRSwapchainExtensionName};

		constexpr std::vector<const char*> optionalExtensions = {};

		m_device.emplace(m_instance, m_surface, requiredExtensions, optionalExtensions);

		if (!m_device->pickPhysicalDevice())
		{
			std::println("failed to pick a physical device!");
			m_hasInitialized = false;
			return false;
		}

		if (!m_device->createLogicalDevice(ENABLE_VALIDATION_LAYERS, VALIDATION_LAYERS))
		{
			std::println("failed to create logical device!");
			m_hasInitialized = false;
			return false;
		}

		m_msaaSamples = getMaxUsableSampleCount(m_device->getPhysicalDevice());
		return true;
	}

	bool Renderer::createSwapChain()
	{
		if (m_hasInitialized)
			return true;

		m_swapChain.emplace(*m_device, m_window.get());

		if (!m_swapChain->create())
		{
			std::println("failed to create swapchain");
			m_hasInitialized = false;
			return false;
		}

		if (!m_swapChain->createImageViews())
		{
			std::println("failed to create swapChainImageViews");
			m_hasInitialized = false;
			return false;
		}

		return true;
	}

	bool Renderer::createUnlitDescriptorSetLayout()
	{
		if (m_hasInitialized)
			return true;

		vk::DescriptorSetLayoutBinding uboLayout{
		    .binding         = 0,
		    .descriptorType  = vk::DescriptorType::eUniformBuffer,
		    .descriptorCount = 1,
		    .stageFlags      = vk::ShaderStageFlagBits::eVertex};

		vk::DescriptorSetLayoutBinding textureLayout{
		    .binding         = 1,
		    .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
		    .descriptorCount = 1,
		    .stageFlags      = vk::ShaderStageFlagBits::eFragment};

		std::array<vk::DescriptorSetLayoutBinding, 2> bindings{{uboLayout, textureLayout}};

		vk::DescriptorSetLayoutCreateInfo layoutInfo{
		    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
		    .pBindings    = bindings.data()};

		m_unlitPipeline.descriptorSetLayout = vk::raii::DescriptorSetLayout(m_device->getDevice(), layoutInfo);
		return true;
	}

	bool Renderer::createUnlitGraphicsPipeline()
	{
		if (m_hasInitialized)
			return true;

		VkShader unlitShader(*m_device);

		if (!unlitShader.createFromFile("resources/shaders/Unlit.spv"))
		{
			std::println("failed to load unlit shader");
			return false;
		}

		unlitShader.addStage(vk::ShaderStageFlagBits::eVertex, "vertMain");
		unlitShader.addStage(vk::ShaderStageFlagBits::eFragment, "fragMain");

		auto& unlitShaderStages = unlitShader.getShaderStages();

		auto bindingDescription   = Vertex::getBindingDescription();
		auto attributeDescription = Vertex::getAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
		    .vertexBindingDescriptionCount   = 1,
		    .pVertexBindingDescriptions      = &bindingDescription,
		    .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescription.size()),
		    .pVertexAttributeDescriptions    = attributeDescription.data()};

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
		    .topology = vk::PrimitiveTopology::eTriangleList};

		vk::PipelineViewportStateCreateInfo viewportState{
		    .viewportCount = 1,
		    .scissorCount  = 1};

		vk::PipelineRasterizationStateCreateInfo rasterizer{
		    .depthClampEnable        = vk::False,
		    .rasterizerDiscardEnable = vk::False,
		    .polygonMode             = vk::PolygonMode::eFill,
		    .cullMode                = vk::CullModeFlagBits::eBack,
		    .frontFace               = vk::FrontFace::eCounterClockwise,
		    .lineWidth               = 1.0F};

		vk::PipelineMultisampleStateCreateInfo multisampling{
		    .rasterizationSamples = m_msaaSamples,
		    .sampleShadingEnable  = vk::False};

		vk::PipelineDepthStencilStateCreateInfo depthStencil{
		    .depthTestEnable       = vk::True,
		    .depthWriteEnable      = vk::True,
		    .depthCompareOp        = vk::CompareOp::eLess,
		    .depthBoundsTestEnable = vk::False,
		    .stencilTestEnable     = vk::False};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		    .blendEnable    = vk::False,
		    .colorWriteMask = {
		        vk::ColorComponentFlagBits::eR |
		        vk::ColorComponentFlagBits::eG |
		        vk::ColorComponentFlagBits::eB |
		        vk::ColorComponentFlagBits::eA}};

		vk::PipelineColorBlendStateCreateInfo colorBlending{
		    .logicOpEnable   = vk::False,
		    .logicOp         = vk::LogicOp::eCopy,
		    .attachmentCount = 1,
		    .pAttachments    = &colorBlendAttachment};

		std::vector<vk::DynamicState>      dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamicState{
		    .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
		    .pDynamicStates    = dynamicStates.data()};

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		    .setLayoutCount         = 1,
		    .pSetLayouts            = &*m_unlitPipeline.descriptorSetLayout,
		    .pushConstantRangeCount = 0};

		m_unlitPipeline.pipelineLayout = vk::raii::PipelineLayout(m_device->getDevice(), pipelineLayoutCreateInfo);

		vk::Format depthFormat = findDepthFormat(m_device->getPhysicalDevice());

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		    .stageCount          = static_cast<std::uint32_t>(unlitShaderStages.size()),
		    .pStages             = unlitShaderStages.data(),
		    .pVertexInputState   = &vertexInputInfo,
		    .pInputAssemblyState = &inputAssembly,
		    .pViewportState      = &viewportState,
		    .pRasterizationState = &rasterizer,
		    .pMultisampleState   = &multisampling,
		    .pDepthStencilState  = &depthStencil,
		    .pColorBlendState    = &colorBlending,
		    .pDynamicState       = &dynamicState,
		    .layout              = m_unlitPipeline.pipelineLayout,
		    .renderPass          = nullptr};

		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
		    .colorAttachmentCount    = 1,
		    .pColorAttachmentFormats = &m_swapChain->getSwapChainSurfaceFormat().format,
		    .depthAttachmentFormat   = depthFormat};

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo>
		    pipelineCreateInfoChain = {graphicsPipelineCreateInfo, pipelineRenderingCreateInfo};

		m_unlitPipeline.pipeline = vk::raii::Pipeline(m_device->getDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
		return true;
	}

	bool Renderer::createLightingDescriptorSetLayout()
	{
		if (m_hasInitialized)
			return true;

		vk::DescriptorSetLayoutBinding uboLayout{
		    .binding         = 0,
		    .descriptorType  = vk::DescriptorType::eUniformBuffer,
		    .descriptorCount = 1,
		    .stageFlags      = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment};

		vk::DescriptorSetLayoutBinding textureLayout{
		    .binding         = 1,
		    .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
		    .descriptorCount = 1,
		    .stageFlags      = vk::ShaderStageFlagBits::eFragment};

		std::array<vk::DescriptorSetLayoutBinding, 2> bindings{{uboLayout, textureLayout}};

		vk::DescriptorSetLayoutCreateInfo layoutInfo{
		    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
		    .pBindings    = bindings.data()};

		m_lightingPipeline.descriptorSetLayout = vk::raii::DescriptorSetLayout(m_device->getDevice(), layoutInfo);
		return true;
	}

	bool Renderer::createLightingGraphicsPipeline()
	{
		if (m_hasInitialized)
			return true;

		VkShader lightingShader(*m_device);

		if (!lightingShader.createFromFile("resources/shaders/BasicLighting.spv"))
		{
			std::println("failed to load basic lighting shader");
			return false;
		}

		lightingShader.addStage(vk::ShaderStageFlagBits::eVertex, "vertMain");
		lightingShader.addStage(vk::ShaderStageFlagBits::eFragment, "fragMain");

		auto& shaderStages = lightingShader.getShaderStages();

		auto bindingDescription   = Vertex::getBindingDescription();
		auto attributeDescription = Vertex::getAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
		    .vertexBindingDescriptionCount   = 1,
		    .pVertexBindingDescriptions      = &bindingDescription,
		    .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescription.size()),
		    .pVertexAttributeDescriptions    = attributeDescription.data()};

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
		    .topology = vk::PrimitiveTopology::eTriangleList};

		vk::PipelineViewportStateCreateInfo viewportState{
		    .viewportCount = 1,
		    .scissorCount  = 1};

		vk::PipelineRasterizationStateCreateInfo rasterizer{
		    .depthClampEnable        = vk::False,
		    .rasterizerDiscardEnable = vk::False,
		    .polygonMode             = vk::PolygonMode::eFill,
		    .cullMode                = vk::CullModeFlagBits::eBack,
		    .frontFace               = vk::FrontFace::eCounterClockwise,
		    .lineWidth               = 1.0F};

		vk::PipelineMultisampleStateCreateInfo multisampling{
		    .rasterizationSamples = m_msaaSamples,
		    .sampleShadingEnable  = vk::False};

		vk::PipelineDepthStencilStateCreateInfo depthStencil{
		    .depthTestEnable       = vk::True,
		    .depthWriteEnable      = vk::True,
		    .depthCompareOp        = vk::CompareOp::eLess,
		    .depthBoundsTestEnable = vk::False,
		    .stencilTestEnable     = vk::False};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		    .blendEnable    = vk::False,
		    .colorWriteMask = {
		        vk::ColorComponentFlagBits::eR |
		        vk::ColorComponentFlagBits::eG |
		        vk::ColorComponentFlagBits::eB |
		        vk::ColorComponentFlagBits::eA}};

		vk::PipelineColorBlendStateCreateInfo colorBlending{
		    .logicOpEnable   = vk::False,
		    .logicOp         = vk::LogicOp::eCopy,
		    .attachmentCount = 1,
		    .pAttachments    = &colorBlendAttachment};

		std::vector<vk::DynamicState> dynamicStates = {
		    vk::DynamicState::eViewport,
		    vk::DynamicState::eScissor};

		vk::PipelineDynamicStateCreateInfo dynamicState{
		    .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
		    .pDynamicStates    = dynamicStates.data()};

		vk::PushConstantRange pushConstantRange{
		    .stageFlags = vk::ShaderStageFlagBits::eFragment,
		    .offset     = 0,
		    .size       = sizeof(Material)};

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		    .setLayoutCount         = 1,
		    .pSetLayouts            = &*m_lightingPipeline.descriptorSetLayout,
		    .pushConstantRangeCount = 1,
		    .pPushConstantRanges    = &pushConstantRange};

		m_lightingPipeline.pipelineLayout = vk::raii::PipelineLayout(m_device->getDevice(), pipelineLayoutCreateInfo);

		vk::Format depthFormat = findDepthFormat(m_device->getPhysicalDevice());

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		    .stageCount          = static_cast<std::uint32_t>(shaderStages.size()),
		    .pStages             = shaderStages.data(),
		    .pVertexInputState   = &vertexInputInfo,
		    .pInputAssemblyState = &inputAssembly,
		    .pViewportState      = &viewportState,
		    .pRasterizationState = &rasterizer,
		    .pMultisampleState   = &multisampling,
		    .pDepthStencilState  = &depthStencil,
		    .pColorBlendState    = &colorBlending,
		    .pDynamicState       = &dynamicState,
		    .layout              = m_lightingPipeline.pipelineLayout,
		    .renderPass          = nullptr};

		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
		    .colorAttachmentCount    = 1,
		    .pColorAttachmentFormats = &m_swapChain->getSwapChainSurfaceFormat().format,
		    .depthAttachmentFormat   = depthFormat};

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo>
		    pipelineCreateInfoChain = {graphicsPipelineCreateInfo, pipelineRenderingCreateInfo};

		m_lightingPipeline.pipeline = vk::raii::Pipeline(m_device->getDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
		return true;
	}

	bool Renderer::createCommandPool()
	{
		if (m_hasInitialized)
			return true;

		auto queueFamilyIndex = m_device->getQueueFamiltIndices();

		vk::CommandPoolCreateInfo poolInfo{
		    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		    .queueFamilyIndex = queueFamilyIndex.graphicsFamily.value()};

		m_commandPool = vk::raii::CommandPool(m_device->getDevice(), poolInfo);
		return true;
	}

	bool Renderer::createColorResources()
	{
		if (m_hasInitialized)
			return true;

		vk::Format colorFormat     = m_swapChain->getSwapChainSurfaceFormat().format;
		auto       swapChainExtent = m_swapChain->getSwapChainExtent();

		std::tie(m_colorImage, m_colorImageMemory) = createImage(m_device->getDevice(), m_device->getPhysicalDevice(),
		    swapChainExtent.width, swapChainExtent.height, 1, m_msaaSamples, colorFormat, vk::ImageTiling::eOptimal,
		    vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);

		m_colorImageView = createImageView(m_device->getDevice(), m_colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
		return true;
	}

	bool Renderer::createDepthResources()
	{
		if (m_hasInitialized)
			return true;

		vk::Format depthFormat     = findDepthFormat(m_device->getPhysicalDevice());
		auto       swapChainExtent = m_swapChain->getSwapChainExtent();

		std::tie(m_depthImage, m_depthImageMemory) = createImage(m_device->getDevice(), m_device->getPhysicalDevice(), swapChainExtent.width,
		    swapChainExtent.height, 1, m_msaaSamples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
		    vk::MemoryPropertyFlagBits::eDeviceLocal);

		m_depthImageView = createImageView(m_device->getDevice(), m_depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
		return true;
	}

	bool Renderer::createCommandBuffers()
	{
		if (m_hasInitialized)
			return true;

		vk::CommandBufferAllocateInfo allocInfo{
		    .commandPool        = m_commandPool,
		    .level              = vk::CommandBufferLevel::ePrimary,
		    .commandBufferCount = RMAX_FRAMES_IN_FLIGHT};

		m_commandBuffers = vk::raii::CommandBuffers(m_device->getDevice(), allocInfo);
		return true;
	}

	bool Renderer::createDescriptorManager()
	{
		if (m_hasInitialized)
			return true;

		const std::uint32_t maxObjectCount = 64;
		m_descriptorManager.emplace(*m_device, m_lightingPipeline.descriptorSetLayout, RMAX_FRAMES_IN_FLIGHT, maxObjectCount);

		return true;
	}

	bool Renderer::createSyncObjects()
	{
		auto& device = m_device->getDevice();

		for (std::size_t i = 0; i < m_swapChain->getSwapChainImages().size(); i++)
		{
			m_renderFinishedSemaphore.emplace_back(device, vk::SemaphoreCreateInfo{});
		}

		for (std::size_t i = 0; i < RMAX_FRAMES_IN_FLIGHT; i++)
		{
			m_presentCompletedSemaphore.emplace_back(device, vk::SemaphoreCreateInfo{});
			m_inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
		}

		return true;
	}

#pragma endregion

#pragma region Rendering

	void Renderer::setFramebufferSize(const std::uint32_t width, const std::uint32_t height)
	{
		m_width  = width;
		m_height = height;

		m_window.get().width  = static_cast<int>(m_width);
		m_window.get().height = static_cast<int>(m_height);
	}

	void Renderer::addRenderObject(RenderObject* renderObject)
	{
		renderObject->objId = m_renderObjectId;

		if (renderObject->model != nullptr)
			renderObject->model->initModel(*m_device, m_commandPool, m_device->getGraphicsQueue());
		else
			std::println("no model attached");

		if (renderObject->model != nullptr)
		{
			for (auto& mesh : renderObject->model->getMesh())
			{
				auto* texture = mesh.getTexture();
				if (texture == nullptr)
					continue;

				mesh.setGpuResource(m_descriptorManager->createObject(texture->getTextureImageView(),
				    texture->getTextureImageSampler()));
			}
		}
		else
			std::println("no texture attached");

		auto [iter, sucess] = m_renderObjects.try_emplace(m_renderObjectId, renderObject);
		auto& object        = iter->second;

		if (sucess)
			m_renderObjectId++;
	}

	void Renderer::removeRenderObject(RenderObject* renderObject)
	{
		auto object = m_renderObjects.find(renderObject->objId);
		if (object == m_renderObjects.end())
			return;

		renderObject->hasBeenDestroyed = true;
		for (auto& mesh : renderObject->model->getMesh())
			m_descriptorManager->releaseObject(mesh.getGpuResources());

		m_renderObjects.erase(object->first);
	}

	Model Renderer::createModel(const std::string& path)
	{
		ModelLoader loadModel{};
		loadModel.parseModel(path);
		return std::move(loadModel.getModel());
	}

	Texture Renderer::createTexture(const std::string& path, bool useMipMaps)
	{
		Texture texture(m_device.value(), m_commandPool, m_device->getGraphicsQueue());
		texture.createFromFile(path, useMipMaps);
		return std::move(texture);
	}

	void Renderer::recordCommandBuffer(std::uint32_t imageIndex)
	{
		auto& commandBuffer = m_commandBuffers[m_frameIndex];
		commandBuffer.begin({});

		const auto& swapChainImage     = m_swapChain->getSwapChainImages().at(imageIndex);
		const auto& swapChainImageView = m_swapChain->getSwapChainImageViews().at(imageIndex);

		transitionImageLayout(
		    commandBuffer,
		    swapChainImage,
		    vk::ImageLayout::eUndefined,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    {},
		    vk::AccessFlagBits2::eColorAttachmentWrite,
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		    vk::ImageAspectFlagBits::eColor);

		transitionImageLayout(
		    commandBuffer,
		    *m_colorImage,
		    vk::ImageLayout::eUndefined,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    vk::AccessFlagBits2::eColorAttachmentWrite,
		    vk::AccessFlagBits2::eColorAttachmentWrite,
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		    vk::ImageAspectFlagBits::eColor);

		transitionImageLayout(
		    commandBuffer,
		    *m_depthImage,
		    vk::ImageLayout::eUndefined,
		    vk::ImageLayout::eDepthAttachmentOptimal,
		    vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		    vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		    vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		    vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		    vk::ImageAspectFlagBits::eDepth);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0F, 0.0F, 0.0F, 1.0F);
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0F, 0);

		vk::RenderingAttachmentInfo colorAttachmentInfo{
		    .imageView          = m_colorImageView,
		    .imageLayout        = vk::ImageLayout::eColorAttachmentOptimal,
		    .resolveMode        = vk::ResolveModeFlagBits::eAverage,
		    .resolveImageView   = swapChainImageView,
		    .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		    .loadOp             = vk::AttachmentLoadOp::eClear,
		    .storeOp            = vk::AttachmentStoreOp::eStore,
		    .clearValue         = clearColor};

		vk::RenderingAttachmentInfo depthAttachmentInfo{
		    .imageView   = m_depthImageView,
		    .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		    .loadOp      = vk::AttachmentLoadOp::eClear,
		    .storeOp     = vk::AttachmentStoreOp::eDontCare,
		    .clearValue  = clearDepth};

		auto swapChainExtent = m_swapChain->getSwapChainExtent();

		vk::RenderingInfo renderingInfo{
		    .renderArea = {
		        .offset = {.x = 0, .y = 0},
		        .extent = swapChainExtent},
		    .layerCount           = 1,
		    .colorAttachmentCount = 1,
		    .pColorAttachments    = &colorAttachmentInfo,
		    .pDepthAttachment     = &depthAttachmentInfo};

		commandBuffer.beginRendering(renderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_lightingPipeline.pipeline);

		commandBuffer.setViewport(0,
		    vk::Viewport(
		        0.0F,
		        static_cast<float>(swapChainExtent.height),
		        static_cast<float>(swapChainExtent.width),
		        -static_cast<float>(swapChainExtent.height),
		        0.0F, 1.0F));

		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

		for (auto& object : m_renderObjects)
		{
			auto& renderObject = object.second;
			if (renderObject->shouldBeDestroyed) continue;

			for (auto& mesh : renderObject->model->getMesh())
			{
				auto& vertexBuffer = mesh.getVertexBuffer();
				auto& indexBuffer  = mesh.getIndexBuffer();

				auto indicies = mesh.getIndicies();

				commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
				commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexTypeValue<decltype(indicies)::value_type>::value);

				Material mat = mesh.getMaterial();
				pushMaterialProperties(commandBuffer, mat);

				VkDescriptorManager::bindObjectForDraw(commandBuffer, m_lightingPipeline.pipelineLayout, mesh.getGpuResources(), m_frameIndex);
				commandBuffer.drawIndexed(indicies.size(), 1, 0, 0, 0);
			}
		}

		commandBuffer.endRendering();

		transitionImageLayout(
		    commandBuffer,
		    swapChainImage,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    vk::ImageLayout::ePresentSrcKHR,
		    vk::AccessFlagBits2::eColorAttachmentWrite,
		    {},
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		    vk::PipelineStageFlagBits2::eBottomOfPipe,
		    vk::ImageAspectFlagBits::eColor);

		commandBuffer.end();
	}

	void Renderer::updateUniformBuffers(Camera3D& camera)
	{
		for (auto& object : m_renderObjects)
		{
			auto& renderObject = object.second;
			if (renderObject->shouldBeDestroyed) continue;

			UniformBufferObject ubo{
			    .model = renderObject->transform.getMatrix(),
			    .view  = camera.getViewMatrix(),
			    .proj  = camera.getProjectionMatrix(
			        static_cast<float>(m_swapChain->getSwapChainExtent().width) /
			        static_cast<float>(m_swapChain->getSwapChainExtent().height)),
			    .camPos     = glm::vec4(camera.getPosition(), 1.0F),
			    .lightCount = 0};

			for (auto& mesh : renderObject->model->getMesh())
				VkDescriptorManager::updateObjectForFrame(mesh.getGpuResources(), m_frameIndex, ubo);
		}
	}

	void Renderer::onFramebufferResized()
	{
		m_swapChain->recreate();
		createColorResources();
		createDepthResources();
	}

	void Renderer::pushMaterialProperties(vk::CommandBuffer commandBuffer, const Material& material) const
	{
		commandBuffer.pushConstants(*m_lightingPipeline.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(Material), &material);
	}

	void Renderer::destroyOldObjects()
	{
		std::vector<RenderObject*> toRemove;

		for (auto& object : m_renderObjects)
		{
			auto& renderObject = object.second;

			if (!renderObject->shouldBeDestroyed) continue;
			if (renderObject->framesPassedBeforeDestroy < RMAX_FRAMES_IN_FLIGHT + 1)
			{
				renderObject->framesPassedBeforeDestroy++;
				continue;
			}

			toRemove.push_back(renderObject);
		}

		for (auto& object : toRemove)
		{
			removeRenderObject(object);
		}
	}

	void Renderer::updateFrameTime()
	{
		float now   = static_cast<float>(glfwGetTime());
		m_deltaTime = now - m_lastFrame;
		m_lastFrame = now;
	}

	void Renderer::render(Camera3D& camera)
	{
		updateFrameTime();
		destroyOldObjects();

		auto& commandBuffer = m_commandBuffers[m_frameIndex];

		auto& inFlightFence            = m_inFlightFences[m_frameIndex];
		auto& presentCompleteSemaphore = m_presentCompletedSemaphore[m_frameIndex];

		auto& device      = m_device->getDevice();
		auto  fenceResult = device.waitForFences(*inFlightFence, vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			std::println("failed to wait for fence");
			return;
		}

		auto [result, imageIndex] = m_swapChain->getSwapChain().acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);
		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			onFramebufferResized();
			return;
		}

		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			return;
		}

		device.resetFences(*inFlightFence);

		commandBuffer.reset();
		recordCommandBuffer(imageIndex);

		updateUniformBuffers(camera);

		auto& renderFinishedSemaphore = m_renderFinishedSemaphore[imageIndex];

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::SubmitInfo   submitInfo{
		    .waitSemaphoreCount   = 1,
		    .pWaitSemaphores      = &*presentCompleteSemaphore,
		    .pWaitDstStageMask    = &waitDestinationStageMask,
		    .commandBufferCount   = 1,
		    .pCommandBuffers      = &*commandBuffer,
		    .signalSemaphoreCount = 1,
		    .pSignalSemaphores    = &*renderFinishedSemaphore};

		m_device->getGraphicsQueue().submit(submitInfo, *inFlightFence);

		const vk::PresentInfoKHR presentInfo{
		    .waitSemaphoreCount = 1,
		    .pWaitSemaphores    = &*renderFinishedSemaphore,
		    .swapchainCount     = 1,
		    .pSwapchains        = &*m_swapChain->getSwapChain(),
		    .pImageIndices      = &imageIndex};

		result = m_device->getGraphicsQueue().presentKHR(presentInfo);
		if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || m_window.get().hasFramebufferResized)
		{
			m_window.get().hasFramebufferResized = false;
			onFramebufferResized();
		}

		m_frameIndex = (m_frameIndex + 1) % RMAX_FRAMES_IN_FLIGHT;
	}

#pragma endregion

	void Renderer::cleanup()
	{
		m_device->getDevice().waitIdle();
		for (auto& rObject : m_renderObjects)
		{
			auto [index, obj] = rObject;
			for (auto& mesh : obj->model->getMesh())
				m_descriptorManager->releaseObject(mesh.getGpuResources());
		}

		m_resourceManager.clear();
	}

} // namespace erb
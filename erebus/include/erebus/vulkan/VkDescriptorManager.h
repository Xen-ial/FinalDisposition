#pragma once
#include <erebus/vulkan/RenderObject.h>

#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <functional>
#include <cstdint>

namespace erb
{
	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model = glm::mat4(1.0F);
		alignas(16) glm::mat4 view  = glm::mat4(1.0F);
		alignas(16) glm::mat4 proj  = glm::mat4(1.0F);
		alignas(16) glm::vec4 camPos = glm::vec4(1.0F);

		alignas(16) glm::int32_t lightCount = 0;
	};

	struct PoolRecord
	{
		vk::raii::DescriptorPool pool{nullptr};

		std::uint32_t maxSets{};
		std::uint32_t uniformCapacity{};
		std::uint32_t samplerCapacity{};

		std::uint32_t liveSets{};
		std::uint32_t liveUniformDescriptors{};
		std::uint32_t liveSamplerDescriptors{};
	};

	class VkDescriptorManager
	{
	public:
		VkDescriptorManager(VkDevice& device, vk::raii::DescriptorSetLayout& objectLayout,
		    std::uint32_t framesInFlight = 2, std::uint32_t objectsPerPool = 64)
		    : m_device(device), m_layout(objectLayout), m_framesInFlight(framesInFlight), m_objectsPerPool(objectsPerPool)
		{
			if (m_framesInFlight == 0 || m_objectsPerPool == 0)
				throw std::invalid_argument("failed to create VkDescriptorManager, objectPool or framesInFlight must be more than 0");
		}

		ObjectGpuResources createObject(const vk::raii::ImageView& imageView,
		    const vk::raii::Sampler&                               sampler);

		void releaseObject(ObjectGpuResources& object);

		static void updateObjectForFrame(ObjectGpuResources& gpu, std::uint32_t frame, const UniformBufferObject& ubo);
		static void bindObjectForDraw(vk::raii::CommandBuffer& commandBuffer, const vk::raii::PipelineLayout& pipelineLayout,
		    const ObjectGpuResources& gpu, std::uint32_t frame);

	private:
		std::reference_wrapper<VkDevice>                      m_device;
		std::reference_wrapper<vk::raii::DescriptorSetLayout> m_layout;

		std::uint32_t m_framesInFlight{};
		std::uint32_t m_objectsPerPool{};

		std::vector<PoolRecord> m_pools;

		std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createUniformBuffer();

		PoolRecord makePool();

		[[nodiscard]] bool canFitObject(const PoolRecord& pool) const;

		std::uint32_t findPoolForObject();
	};
} // namespace erb
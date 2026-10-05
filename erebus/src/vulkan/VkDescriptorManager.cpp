#include "vulkan/vulkan.hpp"
#include <erebus/vulkan/VkDescriptorManager.h>
#include <erebus/vulkan/RenderObject.h>

#include <vulkan/vulkan_raii.hpp>

#include <print>

namespace erb
{
	ObjectGpuResources VkDescriptorManager::createObject(const vk::raii::ImageView& imageView,
	    const vk::raii::Sampler&                                                    sampler)
	{
		ObjectGpuResources result;
		result.descriptorCountPerType = m_framesInFlight;

		for (std::uint32_t frame = 0; frame < m_framesInFlight; frame++)
		{
			auto [buffer, memory] = createUniformBuffer();
			void* mapped          = memory.mapMemory(0, sizeof(UniformBufferObject));

			result.uniformBuffers.push_back(std::move(buffer));
			result.uniformBuffersMemory.push_back(std::move(memory));
			result.mappedUniforms.push_back(mapped);
		}

		result.poolIndex  = findPoolForObject();
		PoolRecord& owner = m_pools.at(result.poolIndex);

		std::vector<vk::DescriptorSetLayout> layouts(m_framesInFlight, *m_layout.get());
		vk::DescriptorSetAllocateInfo        allocateInfo{
		    .descriptorPool     = *owner.pool,
		    .descriptorSetCount = m_framesInFlight,
		    .pSetLayouts        = layouts.data()};

		result.descriptorSets = m_device.get().getDevice().allocateDescriptorSets(allocateInfo);

		owner.liveSets += m_framesInFlight;
		owner.liveUniformDescriptors += m_framesInFlight;
		owner.liveSamplerDescriptors += m_framesInFlight;

		for (std::uint32_t frame = 0; frame < m_framesInFlight; frame++)
		{
			vk::DescriptorBufferInfo bufferInfo{
			    .buffer = *result.uniformBuffers[frame],
			    .offset = 0,
			    .range  = sizeof(UniformBufferObject)};

			vk::DescriptorImageInfo imageInfo{
			    .sampler     = *sampler,
			    .imageView   = *imageView,
			    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

			std::array<vk::WriteDescriptorSet, 2> writes{
			    {vk::WriteDescriptorSet{
			         .dstSet          = *result.descriptorSets[frame],
			         .dstBinding      = 0,
			         .dstArrayElement = 0,
			         .descriptorCount = 1,
			         .descriptorType  = vk::DescriptorType::eUniformBuffer,
			         .pBufferInfo     = &bufferInfo},
			        vk::WriteDescriptorSet{
			            .dstSet          = *result.descriptorSets[frame],
			            .dstBinding      = 1,
			            .dstArrayElement = 0,
			            .descriptorCount = 1,
			            .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
			            .pImageInfo      = &imageInfo}}};

			m_device.get().getDevice().updateDescriptorSets(writes, {});
		}

		return result;
	}

	void VkDescriptorManager::releaseObject(ObjectGpuResources& object)
	{
		if (object.descriptorSets.empty())
			return;

		PoolRecord&         owner = m_pools.at(object.poolIndex);
		const std::uint32_t count = object.descriptorCountPerType;

		assert(owner.liveSets >= count);
		assert(owner.liveUniformDescriptors >= count);
		assert(owner.liveSamplerDescriptors >= count);

		object.descriptorSets.clear();

		owner.liveSets -= count;
		owner.liveUniformDescriptors -= count;
		owner.liveSamplerDescriptors -= count;

		for (auto& memory : object.uniformBuffersMemory)
			memory.unmapMemory();

		object.mappedUniforms.clear();
		object.uniformBuffers.clear();
		object.uniformBuffersMemory.clear();

		object.descriptorCountPerType = 0;
	}

	void VkDescriptorManager::updateObjectForFrame(ObjectGpuResources& gpu, std::uint32_t frame, const UniformBufferObject& ubo)
	{
		std::memcpy(gpu.mappedUniforms.at(frame), &ubo, sizeof(ubo));
	}

	void VkDescriptorManager::bindObjectForDraw(vk::raii::CommandBuffer& commandBuffer, const vk::raii::PipelineLayout& pipelineLayout,
	    const ObjectGpuResources& gpu, std::uint32_t frame)
	{
		vk::DescriptorSet set = *gpu.descriptorSets.at(frame);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		    *pipelineLayout, 0, set, {});
	}

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> VkDescriptorManager::createUniformBuffer()
	{
		auto& device = m_device.get();

		vk::BufferCreateInfo bufferInfo{
		    .size        = sizeof(UniformBufferObject),
		    .usage       = vk::BufferUsageFlagBits::eUniformBuffer,
		    .sharingMode = vk::SharingMode::eExclusive};

		vk::raii::Buffer             buffer(device.getDevice(), bufferInfo);
		const vk::MemoryRequirements requirements = buffer.getMemoryRequirements();

		vk::MemoryAllocateInfo memoryInfo{
		    .allocationSize  = requirements.size,
		    .memoryTypeIndex = device.findMemoryType(requirements.memoryTypeBits,
		        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)};

		vk::raii::DeviceMemory memory(device.getDevice(), memoryInfo);
		buffer.bindMemory(*memory, 0);

		return {std::move(buffer), std::move(memory)};
	}

	PoolRecord VkDescriptorManager::makePool()
	{
		const std::uint32_t setCapacity = m_objectsPerPool * m_framesInFlight;

		std::array<vk::DescriptorPoolSize, 2> sizes{
		    {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = setCapacity},
		        {.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = setCapacity}}};

		vk::DescriptorPoolCreateInfo createInfo{
		    .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		    .maxSets       = setCapacity,
		    .poolSizeCount = static_cast<std::uint32_t>(sizes.size()),
		    .pPoolSizes    = sizes.data()};

		PoolRecord record{};

		record.pool            = vk::raii::DescriptorPool(m_device.get().getDevice(), createInfo);
		record.maxSets         = setCapacity;
		record.uniformCapacity = setCapacity;
		record.samplerCapacity = setCapacity;

		return record;
	}

	bool VkDescriptorManager::canFitObject(const PoolRecord& pool) const
	{
		bool canFitLiveSets               = pool.liveSets + m_framesInFlight <= pool.maxSets;
		bool canFitLiveUniformDescriptors = pool.liveUniformDescriptors + m_framesInFlight <= pool.uniformCapacity;
		bool canFitLiveSamplerDescriptors = pool.liveSamplerDescriptors + m_framesInFlight <= pool.samplerCapacity;

		return canFitLiveSets && canFitLiveUniformDescriptors && canFitLiveSamplerDescriptors;
	}

	std::uint32_t VkDescriptorManager::findPoolForObject()
	{
		for (std::uint32_t i = 0; i < m_pools.size(); i++)
		{
			if (canFitObject(m_pools[i]))
				return i;
		}

		m_pools.push_back(makePool());
		return static_cast<std::uint32_t>(m_pools.size() - 1);
	}
} // namespace erb
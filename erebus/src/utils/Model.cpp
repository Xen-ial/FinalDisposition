#include <erebus/utils/Model.h>
#include <erebus/vulkan/VkHelpers.h>

#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>

namespace erb
{
#pragma region Mesh

	void Mesh::createBuffers(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue)
	{
		createVertexBuffer(device, commandPool, targetQueue);
		createIndexBuffer(device, commandPool, targetQueue);
	}

	void Mesh::createVertexBuffer(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue)
	{
		vk::DeviceSize bufferSize = sizeof(m_verticies[0]) * m_verticies.size();

		auto [stagingBuffer, stagingBufferMemory] = createBuffer(device.getDevice(), device.getPhysicalDevice(), bufferSize,
		    vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void* stagingData = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(stagingData, m_verticies.data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		std::tie(m_vertexBuffer, m_vertexBufferMemory) = createBuffer(device.getDevice(), device.getPhysicalDevice(), bufferSize,
		    vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer(device.getDevice(), commandPool, targetQueue, stagingBuffer, m_vertexBuffer, bufferSize);
	}

	void Mesh::createIndexBuffer(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue)
	{
		vk::DeviceSize bufferSize = sizeof(m_indicies[0]) * m_indicies.size();

		auto [stagingBuffer, stagingBufferMemory] = createBuffer(device.getDevice(), device.getPhysicalDevice(), bufferSize,
		    vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void* stagingData = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(stagingData, m_indicies.data(), static_cast<std::size_t>(bufferSize));
		stagingBufferMemory.unmapMemory();

		std::tie(m_indexBuffer, m_indexBufferMemory) = createBuffer(device.getDevice(), device.getPhysicalDevice(), bufferSize,
		    vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer(device.getDevice(), commandPool, targetQueue, stagingBuffer, m_indexBuffer, bufferSize);
	}

#pragma endregion

#pragma region Model

	void Model::initModel(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue)
	{
		for (auto& mesh : m_meshes)
			mesh.createBuffers(device, commandPool, targetQueue);
	}

	void Model::setTexture(Texture* texture)
	{
		for (auto& mesh : m_meshes)
			mesh.setTexture(texture);
	}

	void Model::addMesh(Mesh mesh)
	{
		m_meshes.push_back(std::move(mesh));
	}

	void Model::reset()
	{
		m_meshes.clear();
	}

#pragma endregion
} // namespace erb
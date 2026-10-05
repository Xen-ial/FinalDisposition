#pragma once
#include <erebus/utils/Vertex.h>
#include <erebus/vulkan/VkDevice.h>

#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace erb
{
	class Model
	{
	public:
		Model() = default;

		void setVerticies(std::vector<Vertex>& verticies) { m_verticies = verticies; }
		void setIndicies(std::vector<std::uint32_t>& indicies) { m_indicies = indicies; }

		std::vector<Vertex>&        getVerticies() { return m_verticies; }
		std::vector<std::uint32_t>& getIndicies() { return m_indicies; }

		void createBuffers(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);

		vk::raii::Buffer&       getVertexBuffer() { return m_vertexBuffer; }
		vk::raii::DeviceMemory& getVertexBufferMemory() { return m_vertexBufferMemory; }

		vk::raii::Buffer&       getIndexBuffer() { return m_indexBuffer; }
		vk::raii::DeviceMemory& getIndexBufferMemory() { return m_indexBufferMemory; }

	private:
		std::vector<Vertex>        m_verticies;
		std::vector<std::uint32_t> m_indicies;

		vk::raii::Buffer       m_vertexBuffer       = nullptr;
		vk::raii::DeviceMemory m_vertexBufferMemory = nullptr;

		vk::raii::Buffer       m_indexBuffer       = nullptr;
		vk::raii::DeviceMemory m_indexBufferMemory = nullptr;

		void createVertexBuffer(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);
		void createIndexBuffer(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);
	};
} // namespace erb
#pragma once
#include <erebus/utils/Vertex.h>
#include <erebus/utils/Material.h>
#include <erebus/utils/Texture.h>

#include <erebus/vulkan/VkDevice.h>

#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace erb
{
	struct ObjectGpuResources
	{
		std::uint32_t poolIndex{};
		std::uint32_t descriptorCountPerType{};

		std::vector<vk::raii::Buffer>       uniformBuffers;
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
		std::vector<void*>                  mappedUniforms;

		std::vector<vk::raii::DescriptorSet> descriptorSets;
	};

	class Mesh
	{
	public:
		void setVerticies(std::vector<Vertex>& verticies) { m_verticies = verticies; }
		void setIndicies(std::vector<std::uint32_t>& indicies) { m_indicies = indicies; }

		std::vector<Vertex>&        getVerticies() { return m_verticies; }
		std::vector<std::uint32_t>& getIndicies() { return m_indicies; }

		void createBuffers(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);

		vk::raii::Buffer&       getVertexBuffer() { return m_vertexBuffer; }
		vk::raii::DeviceMemory& getVertexBufferMemory() { return m_vertexBufferMemory; }

		vk::raii::Buffer&       getIndexBuffer() { return m_indexBuffer; }
		vk::raii::DeviceMemory& getIndexBufferMemory() { return m_indexBufferMemory; }

		void      setMaterial(const Material& material) { m_material = material; }
		Material& getMaterial() { return m_material; }

		void     setTexture(Texture* texture) { m_texture = texture; }
		Texture* getTexture() { return m_texture; }

	private:
		std::vector<Vertex>        m_verticies;
		std::vector<std::uint32_t> m_indicies;

		vk::raii::Buffer       m_vertexBuffer       = nullptr;
		vk::raii::DeviceMemory m_vertexBufferMemory = nullptr;

		vk::raii::Buffer       m_indexBuffer       = nullptr;
		vk::raii::DeviceMemory m_indexBufferMemory = nullptr;

		Material m_material{};

		Texture* m_texture = nullptr;

		bool m_isInitialized{false};
		bool m_gpuResourceSet{false};

		void createVertexBuffer(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);
		void createIndexBuffer(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);
	};

	class Model
	{
	public:
		Model() = default;

		void initModel(VkDevice& device, vk::raii::CommandPool& commandPool, vk::raii::Queue& targetQueue);

		void setTexture(Texture* texture);
		void addMesh(Mesh mesh);
		void reset();

		[[nodiscard]] const std::vector<Mesh>& getMesh() const { return m_meshes; }
		std::vector<Mesh>&                     getMesh() { return m_meshes; }

	private:
		std::vector<Mesh> m_meshes;
	};
} // namespace erb
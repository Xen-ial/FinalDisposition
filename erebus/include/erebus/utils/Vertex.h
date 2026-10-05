#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace erb
{
	struct Vertex
	{
		glm::vec3 pos      = glm::vec3(1.0F);
		glm::vec3 color    = glm::vec3(1.0F);
		glm::vec2 texCoord = glm::vec2(1.0F);

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			return {
			    .binding   = 0,
			    .stride    = sizeof(Vertex),
			    .inputRate = vk::VertexInputRate::eVertex};
		}

		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
			return {
			    {{.location   = 0,
			         .binding = 0,
			         .format  = vk::Format::eR32G32B32Sfloat,
			         .offset  = offsetof(Vertex, pos)},
			        {.location   = 1,
			            .binding = 0,
			            .format  = vk::Format::eR32G32B32Sfloat,
			            .offset  = offsetof(Vertex, color)},
			        {.location   = 2,
			            .binding = 0,
			            .format  = vk::Format::eR32G32Sfloat,
			            .offset  = offsetof(Vertex, texCoord)}}};
		}

		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};
} // namespace erb

template <>
struct std::hash<erb::Vertex>
{
	std::size_t operator()(const erb::Vertex& vertex) const noexcept
	{
		auto hash = std::hash<glm::vec3>()(vertex.pos) & (std::hash<glm::vec3>()(vertex.color) << 1);

		hash = (hash >> 1) & (std::hash<glm::vec2>()(vertex.texCoord) << 1);
		return hash;
	}
};

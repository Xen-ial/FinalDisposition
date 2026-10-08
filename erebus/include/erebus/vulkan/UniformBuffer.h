#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>

namespace erb
{
	constexpr std::uint32_t MAX_POINT_LIGHTS = 5;
	constexpr std::uint32_t MAX_SPOT_LIGHTS  = 5;

	struct DirectionalLight
	{
		alignas(16) glm::vec4 direction = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		alignas(16) glm::vec4 color     = glm::vec4(1.0F, 1.0F, 1.0F, 2.5F);
	};

	struct PointLight
	{
		alignas(16) glm::vec4 position = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		alignas(16) glm::vec4 color    = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		alignas(16) glm::vec4 params   = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
	};

	struct SpotLight
	{
		alignas(16) glm::vec4 position  = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		alignas(16) glm::vec4 direction = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		alignas(16) glm::vec4 color     = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		alignas(16) glm::vec4 params    = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);

		float outterCos{};
		float innerCos{};
	};

	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model  = glm::mat4(1.0F);
		alignas(16) glm::mat4 view   = glm::mat4(1.0F);
		alignas(16) glm::mat4 proj   = glm::mat4(1.0F);
		alignas(16) glm::vec4 camPos = glm::vec4(1.0F);

		DirectionalLight dirLight{};

		float                                    activePointLightCount{};
		std::array<PointLight, MAX_POINT_LIGHTS> pointLights;

		float                                  activeSpotLightCount{};
		std::array<SpotLight, MAX_SPOT_LIGHTS> spotLights;
	};
} // namespace erb
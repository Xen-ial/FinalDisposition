#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace erb
{
	struct Material
	{
		glm::vec4 ambientColor  = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		glm::vec4 diffuseColor  = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);
		glm::vec4 specularColor = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);

		float shininess = 1.0F;
	};
} // namespace erb
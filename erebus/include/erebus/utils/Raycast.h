#pragma once

#include <erebus/vulkan/RenderObject.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <box3d/box3d.h>
#include <box3d/types.h>
#include <box3d/id.h>

#include <vector>

namespace erb
{
	struct RaycastHit
	{
		RenderObject* renderObject = nullptr;
		glm::vec3     point        = glm::vec3(1.0F);
		glm::vec3     normal       = glm::vec3(1.0F);
		float         fraction{};
		bool          hasHit{false};
	};

	class Raycast
	{
	public:
		Raycast() = default;
		Raycast(b3WorldId& worldId, std::vector<RenderObject>& renderObjects)
		    : m_worldId(&worldId), m_renderObjects(&renderObjects) {}

		RaycastHit castRay(glm::vec3 origin, glm::vec3 translation);

	private:
		b3WorldId*                  m_worldId       = nullptr;
		std::vector<RenderObject>* m_renderObjects = nullptr;
	};
} // namespace erb
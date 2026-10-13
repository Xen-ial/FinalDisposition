#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/types.h"
#include <cstdint>
#include <erebus/utils/Raycast.h>

namespace erb
{
	struct InternalRaycastHit
	{
		b3BodyId  bodyId{};
		glm::vec3 point  = glm::vec3(1.0F);
		glm::vec3 normal = glm::vec3(1.0F);
		float     fraction{};
	};

	float raycastCallback(b3ShapeId shapeId, b3Vec3 point, b3Vec3 normal, float fraction, std::uint64_t userMaterialId,
	    int triangleIndex, int childIndex, void* context)
	{
		InternalRaycastHit* hit = static_cast<InternalRaycastHit*>(context);

		hit->point    = glm::vec3(point.x, point.y, point.z);
		hit->normal   = glm::vec3(normal.x, normal.y, normal.z);
		hit->fraction = fraction;

		b3BodyId bodyId = b3Shape_GetBody(shapeId);
		hit->bodyId     = bodyId;

		return fraction;
	}

	RaycastHit Raycast::castRay(glm::vec3 origin, glm::vec3 translation)
	{
		RaycastHit         raycast{};
		InternalRaycastHit internalHit{};

		b3Vec3 rayOrigin      = {.x = origin.x, .y = origin.y, .z = origin.z};
		b3Vec3 rayTranslation = {.x = translation.x, .y = translation.y, .z = translation.z};

		b3QueryFilter filter = b3DefaultQueryFilter();
		b3World_CastRay(*m_worldId, rayOrigin, rayTranslation, filter, raycastCallback, &internalHit);
		for (auto& object : *m_renderObjects)
		{
			if (!b3Body_IsValid(object.physicsId))
				continue;

			if (object.physicsId.generation == internalHit.bodyId.generation &&
			    object.physicsId.index1 == internalHit.bodyId.index1 &&
			    object.physicsId.world0 == internalHit.bodyId.world0)
			{
				raycast.renderObject = &object;
				raycast.hasHit       = true;
				break;
			}
		}

		if (!raycast.hasHit)
			return {};

		raycast.fraction = internalHit.fraction;
		raycast.normal   = internalHit.normal;
		raycast.point    = internalHit.point;

		return raycast;
	}
} // namespace erb
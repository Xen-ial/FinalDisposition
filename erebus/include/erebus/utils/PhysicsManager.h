#pragma once
#include <erebus/vulkan/RenderObject.h>
#include <erebus/utils/Raycast.h>

#include <box3d/box3d.h>
#include <box3d/id.h>
#include <box3d/collision.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <cstdint>

namespace erb
{
	enum class PhysicsObjectType : std::uint8_t
	{
		STATIC_OBJECT = 0,
		DYNAMIC_OBJECT,
		KINEMATIC_OBJECT
	};

	struct DynamicPhysicsMaterial
	{
		float friction = 0.3F;
		float density  = 1.0F;
		float dampening = -1.0F;
	};

	struct CapsuleCreationInfo
	{
		glm::vec3 center1 = glm::vec3(1.0F, 1.0F, 1.0F);
		glm::vec3 center2 = glm::vec3(1.0F, 1.0F, 1.0F);
		float     radius{};
	};

	class PhysicsManager
	{
	public:
		PhysicsManager() = default;

		void init(std::vector<RenderObject>& renderObjects);
		void cleanup(std::vector<RenderObject>& renderObjects);
		void update(std::vector<RenderObject>& renderObjects, float elapsedTime);

		void        createPhysicsBody(RenderObject& renderObject, PhysicsObjectType objectType);
		static void destroyPhysicsObject(RenderObject& renderObject);

		static void createBoxHullShape(RenderObject& renderObject, const glm::vec3& size, const DynamicPhysicsMaterial& dynamicMaterial = {});
		static void createMeshShape(RenderObject& renderObject, const DynamicPhysicsMaterial& dynamicMaterial = {});
		static void createCapsuleShape(RenderObject& renderObject, const CapsuleCreationInfo& info, const DynamicPhysicsMaterial& dynamicMaterial = {});

		static void setLinearVelocity(RenderObject& renderObject, const glm::vec3& velocity);
		static void addLinearVelocity(RenderObject& renderObject, const glm::vec3& velocity);

		static void addForce(RenderObject& renderObject, const glm::vec3& force);
		static void setForce(RenderObject& renderObject, const glm::vec3& force);

		static void wakeBody(RenderObject& renderObject);

		void setArePhysicsUpdatesEnabled(bool value) { m_isPhysicsUpdatesEnabled = value; }

		b3WorldDef& getWorldDef() { return m_worldDef; }
		b3WorldId&  getworldId() { return m_worldId; }

		float& getFixedTimeStep() { return m_fixedTime; }
		int&   getSubStepCount() { return m_subStepCount; }

		[[nodiscard]] bool isFixedTimeAvailable() const{return m_accumulator >= m_fixedTime;}

		glm::vec3& getGravity()
		{
			return m_gravity;
		}

		[[nodiscard]] const Raycast& getRaycastManager() const { return m_raycastManager; }

	private:
		b3WorldDef m_worldDef{};
		b3WorldId  m_worldId{};

		Raycast m_raycastManager;

		float m_fixedTime{1.0F / 60.0F};
		int   m_subStepCount{4};

		float m_accumulator{};

		bool m_isPhysicsUpdatesEnabled{true};

		glm::vec3 m_gravity{glm::vec3(1.0F)};
	};
} // namespace erb
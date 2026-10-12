#pragma once
#include <erebus/vulkan/RenderObject.h>

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
		DYNAMIC_OBJECT
	};

	struct DynamicPhysicsMaterial
	{
		float friction = 0.3F;
		float density  = 1.0F;
	};

	class PhysicsManager
	{
	public:
		PhysicsManager() = default;

		void init();
		void cleanup(std::vector<RenderObject>& renderObjects);
		void update(std::vector<RenderObject>& renderObjects, float elapsedTime);

		static void destroyPhysicsObject(RenderObject& renderObject);

		void createPhysicsBody(RenderObject& renderObject, PhysicsObjectType objectType);

		static void createBoxHullShape(RenderObject& renderObject, const glm::vec3& size, const DynamicPhysicsMaterial& dynamicMaterial = {});
		static void createMeshShape(RenderObject& renderObject, const DynamicPhysicsMaterial& dynamicMaterial = {});

		void setArePhysicsUpdatesEnabled(bool value) { m_isPhysicsUpdatesEnabled = value; }

		b3WorldDef& getWorldDef() { return m_worldDef; }
		b3WorldId&  getworldId() { return m_worldId; }

		float& getFixedTimeStep() { return m_fixedTime; }
		int&   getSubStepCount() { return m_subStepCount; }

		glm::vec3& getGravity() { return m_gravity; }

	private:
		b3WorldDef m_worldDef{};
		b3WorldId  m_worldId{};

		float m_fixedTime{1.0F / 60.0F};
		int   m_subStepCount{4};

		float m_accumulator{};

		bool m_isPhysicsUpdatesEnabled{true};

		glm::vec3 m_gravity{glm::vec3(1.0F)};
	};
} // namespace erb
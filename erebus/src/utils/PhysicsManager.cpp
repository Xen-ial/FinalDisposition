#include "box3d/collision.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include <erebus/utils/PhysicsManager.h>

#include <box3d/box3d.h>
#include <box3d/types.h>

namespace erb
{
	void PhysicsManager::init()
	{
		m_worldDef         = b3DefaultWorldDef();
		m_worldDef.gravity = {.x = 0.0F, .y = -15.0F, .z = 0.0F};
		m_worldId          = b3CreateWorld(&m_worldDef);
	}

	void PhysicsManager::cleanup(std::vector<RenderObject>& renderObjects)
	{
		for (auto& object : renderObjects)
		{
			for (auto& meshData : object.meshColliderData)
				b3DestroyMesh(meshData.second.meshDataPtr);

			b3DestroyBody(object.physicsId);
			object.physicsId = b3_nullBodyId;
		}

		b3DestroyWorld(m_worldId);
	}

	void PhysicsManager::destroyPhysicsObject(RenderObject& renderObject)
	{
		for (auto& meshData : renderObject.meshColliderData)
			b3DestroyMesh(meshData.second.meshDataPtr);

		b3DestroyBody(renderObject.physicsId);
		renderObject.physicsId = b3_nullBodyId;
	}

	void PhysicsManager::update(std::vector<RenderObject>& renderObjects, const float elapsedTime)
	{
		for (auto& renderObject : renderObjects)
		{
			if (!b3Body_IsValid(renderObject.physicsId))
				continue;

			auto rPos = renderObject.transform.getPosition();
			auto rRot = renderObject.transform.getRotation();

			b3Transform syncTransform{};
			syncTransform.p.x = rPos.x;
			syncTransform.p.y = rPos.y;
			syncTransform.p.z = rPos.z;

			glm::quat rotX = glm::angleAxis(rRot.x, glm::vec3(1.0F, 0.0F, 0.0F));
			glm::quat rotY = glm::angleAxis(rRot.y, glm::vec3(0.0F, 1.0F, 0.0F));
			glm::quat rotZ = glm::angleAxis(rRot.z, glm::vec3(0.0F, 0.0F, 1.0F));

			glm::quat quatRotation = rotZ * rotY * rotX;

			syncTransform.q.v.x = quatRotation.x;
			syncTransform.q.v.y = quatRotation.y;
			syncTransform.q.v.z = quatRotation.z;
			syncTransform.q.s   = quatRotation.w;

			b3Vec3 position = b3Body_GetPosition(renderObject.physicsId);
			b3Quat rotation = b3Body_GetRotation(renderObject.physicsId);

			if ((syncTransform.p.x != position.x || syncTransform.p.y != position.y || syncTransform.p.z != position.z) ||
			    (syncTransform.q.s != rotation.s || syncTransform.q.v.x != rotation.v.x || syncTransform.q.v.y != rotation.v.y || syncTransform.q.v.z != rotation.v.z))
			{
				b3Body_SetTransform(renderObject.physicsId, syncTransform.p, syncTransform.q);
				b3Body_SetAwake(renderObject.physicsId, true);
			}
		}

		if (m_isPhysicsUpdatesEnabled)
		{
			m_accumulator += elapsedTime;
			while (m_accumulator >= m_fixedTime)
			{
				b3World_Step(m_worldId, m_fixedTime, m_subStepCount);
				m_accumulator -= m_fixedTime;
			}
		}

		for (auto& renderObject : renderObjects)
		{
			if (!b3Body_IsValid(renderObject.physicsId))
				continue;

			if (!renderObject.isPhysicsEnabled)
				continue;

			b3Vec3 position = b3Body_GetPosition(renderObject.physicsId);
			b3Quat rotation = b3Body_GetRotation(renderObject.physicsId);

			glm::vec3 newPosition = glm::vec3(
			    position.x,
			    position.y,
			    position.z);

			glm::vec3 newRotation = glm::vec3(
			    rotation.v.x,
			    rotation.v.y,
			    rotation.v.z);

			renderObject.transform.setPosition(newPosition);
			renderObject.transform.setRotation(newRotation);
		}
	}

	void PhysicsManager::createPhysicsBody(RenderObject& renderObject, PhysicsObjectType objectType)
	{
		b3BodyDef bodyDef = b3DefaultBodyDef();
		switch (objectType)
		{
		case PhysicsObjectType::STATIC_OBJECT:
			bodyDef.type = b3_staticBody;
			break;
		case PhysicsObjectType::DYNAMIC_OBJECT:
			bodyDef.type = b3_dynamicBody;
			break;
		}

		auto& transform = renderObject.transform;
		auto  position  = transform.getPosition();
		auto  rotation  = transform.getRotation();

		bodyDef.position = {
		    .x = position.x,
		    .y = position.y,
		    .z = position.z};

		glm::quat rotX = glm::angleAxis(rotation.x, glm::vec3(1.0F, 0.0F, 0.0F));
		glm::quat rotY = glm::angleAxis(rotation.y, glm::vec3(0.0F, 1.0F, 0.0F));
		glm::quat rotZ = glm::angleAxis(rotation.z, glm::vec3(0.0F, 0.0F, 1.0F));

		glm::quat quatRotation = rotZ * rotY * rotX;

		bodyDef.rotation.v = {
		    .x = quatRotation.x,
		    .y = quatRotation.y,
		    .z = quatRotation.z};

		bodyDef.rotation.s = quatRotation.w;
		bodyDef.userData   = &renderObject;

		renderObject.physicsId        = b3CreateBody(m_worldId, &bodyDef);
		renderObject.isPhysicsEnabled = true;
	}

	void PhysicsManager::createBoxHullShape(RenderObject& renderObject, const glm::vec3& size, const DynamicPhysicsMaterial& dynamicMaterial)
	{
		b3BoxHull boxHull = b3MakeBoxHull(size.x, size.y, size.z);

		b3ShapeDef shapeDef            = b3DefaultShapeDef();
		shapeDef.density               = dynamicMaterial.density;
		shapeDef.baseMaterial.friction = dynamicMaterial.friction;

		b3CreateHullShape(renderObject.physicsId, &shapeDef, &boxHull.base);
	}

	void PhysicsManager::createMeshShape(RenderObject& renderObject, const DynamicPhysicsMaterial& dynamicMaterial)
	{
		for (auto& mesh : renderObject.model->getMesh())
		{
			MeshData data{};

			std::vector<b3Vec3>       vertices;
			std::vector<std::int32_t> indicies;

			auto& meshVerts = mesh.getVerticies();
			for (auto& vertex : meshVerts)
			{
				b3Vec3 vert{};
				vert.x = vertex.position.x;
				vert.y = vertex.position.y;
				vert.z = vertex.position.z;
				vertices.push_back(vert);
			}

			auto& meshIndicies = mesh.getIndicies();
			for (auto& index : meshIndicies)
			{
				std::int32_t indx = static_cast<std::int32_t>(index);
				indicies.push_back(indx);
			}

			b3MeshDef def     = {};
			def.vertices      = vertices.data();
			def.vertexCount   = static_cast<int>(vertices.size());
			def.indices       = indicies.data();
			def.triangleCount = static_cast<int>(indicies.size() / 3);
			def.weldVertices  = true;
			def.identifyEdges = true;

			data.meshDataPtr = b3CreateMesh(&def, nullptr, 0);

			b3ShapeDef shapeDef            = b3DefaultShapeDef();
			shapeDef.density               = dynamicMaterial.density;
			shapeDef.baseMaterial.friction = dynamicMaterial.friction;

			b3ShapeId shapeId = b3CreateMeshShape(renderObject.physicsId, &shapeDef, data.meshDataPtr, {.x = 1.0F, .y = 1.0F, .z = 1.0F});
		}
	}
} // namespace erb
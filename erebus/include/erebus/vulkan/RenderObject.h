#pragma once
#include "box3d/id.h"
#include <erebus/utils/Model.h>
#include <erebus/utils/Texture.h>
#include <erebus/utils/Transform.h>
#include <erebus/utils/Material.h>

#include <vulkan/vulkan_raii.hpp>

#include <box3d/box3d.h>

#include <cstdint>
#include <string>

namespace erb
{
	struct MeshData
	{
		b3MeshData* meshDataPtr = nullptr;
	};

	struct RenderObject
	{
		RenderObject(std::uint32_t objId) : objId(objId) {}

		RenderObject(std::string objName) : objName(std::move(objName)) {}

		RenderObject(std::uint32_t objId, std::string& objName)
		    : objId(objId), objName(objName) {}

		RenderObject() = default;

		std::uint32_t objId{};
		std::string   objName;

		Transform transform{};
		Model*    model = nullptr;

		bool shouldBeDestroyed{false};
		bool hasBeenDestroyed{false};
		bool hasModelSet{false};
		bool hasTextureSet{false};

		int framesPassedBeforeDestroy = 0;

		bool     isPhysicsEnabled{false};
		b3BodyId physicsId{};

		std::unordered_map<Mesh*, ObjectGpuResources> meshGpuResources;
		std::unordered_map<Mesh*, MeshData>           meshColliderData;

		void setModel(Model& model)
		{
			this->model = &model;
			hasModelSet = true;
		}

		void setTexture(Texture& texture)
		{
			this->model->setTexture(&texture);
			hasTextureSet = true;
		}

		void createGpuResource(Mesh& mesh, ObjectGpuResources resource)
		{
			meshGpuResources.try_emplace(&mesh, std::move(resource));
		}

		void createMeshColliderData(Mesh& mesh, MeshData data)
		{
			meshColliderData.try_emplace(&mesh, data);
		}

		ObjectGpuResources& getResource(Mesh& mesh)
		{
			return meshGpuResources.at(&mesh);
		}
	};
} // namespace erb
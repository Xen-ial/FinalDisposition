#pragma once
#include <erebus/utils/Model.h>
#include <erebus/utils/Texture.h>
#include <erebus/utils/Transform.h>

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <optional>

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

	struct RenderMaterials
	{
		RenderMaterials(Model* model, Texture* texture)
		    : model(model), texture(texture) {}

		RenderMaterials(Model* model) : model(model) {}
		RenderMaterials(Texture* texture) : texture(texture) {}

		Model*   model   = nullptr;
		Texture* texture = nullptr;
	};

	struct RenderObject
	{
		RenderObject(std::uint32_t objId) : objId(objId) {}

		RenderObject(std::uint32_t objId, RenderMaterials& renderMaterials)
		    : objId(objId), materials(renderMaterials) {}

		RenderObject(std::uint32_t objId, std::string& objName)
		    : objId(objId), objName(objName) {}

		RenderObject(std::uint32_t objId, std::string& objName, RenderMaterials& renderMaterials)
		    : objId(objId), objName(objName), materials(renderMaterials) {}

		RenderObject() = default;

		std::uint32_t objId{};
		std::string   objName;

		Transform transform{};

		std::optional<RenderMaterials> materials;
		ObjectGpuResources             gpu{};

		bool shouldBeDestroyed{false};
		bool hasBeenDestroyed{false};

		int framesPassedBeforeDestroy = 0;
	};
} // namespace erb
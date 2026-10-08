#pragma once
#include <erebus/utils/Model.h>
#include <erebus/utils/Texture.h>
#include <erebus/utils/Transform.h>
#include <erebus/utils/Material.h>

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <string>

namespace erb
{
	struct RenderObject
	{
		RenderObject(std::uint32_t objId) : objId(objId) {}

		RenderObject(std::uint32_t objId, Model* model)
		    : objId(objId), model(model) {}

		RenderObject(std::uint32_t objId, std::string& objName)
		    : objId(objId), objName(objName) {}

		RenderObject(std::uint32_t objId, std::string& objName, Model* model)
		    : objId(objId), objName(objName), model(model) {}

		RenderObject() = default;

		std::uint32_t objId{};
		std::string   objName;

		Transform transform{};
		Model*    model = nullptr;

		void setModel(Model& model) { this->model = &model; }
		void setTexture(Texture& texture) const { this->model->setTexture(&texture); }

		bool shouldBeDestroyed{false};
		bool hasBeenDestroyed{false};

		int framesPassedBeforeDestroy = 0;
	};
} // namespace erb
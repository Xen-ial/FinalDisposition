#pragma once

#include <erebus/utils/Model.h>
#include <erebus/utils/Texture.h>

#include <string>
#include <map>

namespace erb
{
	struct ResourceManager
	{
		std::map<std::string, Texture> namedTextures;
		std::map<std::string, Model>   namedModels;

		Texture* getTexture(const std::string& resName);
		Texture* addTexture(const std::string& resName, Texture& texture);

		void removeTexture(const std::string& resName);

		Model* getModel(const std::string& resName);
		Model* addModel(const std::string& resName, Model& model);

		void removeModel(const std::string& resName);

		void clear();
	};
} // namespace erb
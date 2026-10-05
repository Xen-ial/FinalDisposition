#include <erebus/utils/ResourceManager.h>

#include <print>

namespace erb
{
	Texture* ResourceManager::getTexture(const std::string& resName)
	{
		if (!namedTextures.contains(resName))
		{
			std::println("texture resource: {} not found", resName);
			return nullptr;
		}

		return &namedTextures.at(resName);
	}

	Texture* ResourceManager::addTexture(const std::string& resName, Texture& texture)
	{
		if (namedTextures.contains(resName))
			return &namedTextures.at(resName);

		auto [iter, result] = namedTextures.try_emplace(resName, std::move(texture));
		if (!result)
		{
			std::println("failed to add texture to resources");
			return nullptr;
		}

		return &namedTextures.at(resName);
	}

	void ResourceManager::removeTexture(const std::string& resName)
	{
		if (!namedTextures.contains(resName))
			return;

		namedTextures.erase(resName);
	}

	Model* ResourceManager::getModel(const std::string& resName)
	{
		if (!namedModels.contains(resName))
		{
			std::println("model resource: {} not found", resName);
			return nullptr;
		}

		return &namedModels.at(resName);
	}

	Model* ResourceManager::addModel(const std::string& resName, Model& model)
	{
		if (namedModels.contains(resName))
			return &namedModels.at(resName);

		auto [iter, success] = namedModels.try_emplace(resName, std::move(model));
		if (!success)
		{
			std::println("failed to add resource: model {}", resName);
			return nullptr;
		}

		return &namedModels.at(resName);
	}

	void ResourceManager::removeModel(const std::string& resName)
	{
		if (!namedModels.contains(resName))
			return;

		namedModels.erase(resName);
	}

	void ResourceManager::clear()
	{
		namedModels.clear();
		namedTextures.clear();
	}
} // namespace erb
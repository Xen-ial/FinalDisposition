#include "erebus/vulkan/RenderObject.h"
#include <application/Game.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/constants.hpp>

#include <print>
#include <vector>
#include <algorithm>
#include <iterator>

namespace fd
{
	Game::Game()
	{
		m_app.init("Final Disposition", 800, 600);

		m_app.setOnInitCallback([this]()
		    { onInit(); });

		m_app.setOnUpdateCallback([this]()
		    { onUpdate(); });

		m_app.setOnDrawCallback([this]()
		    { onDraw(); });

		m_app.setOnResizeCallback([this](int width, int height)
		    { onResize(width, height); });

		m_app.setOnCleanupCallback([this]()
		    { onCleanup(); });
	}

	void Game::run()
	{
		m_app.run();
	}

	void Game::onInit()
	{
		auto& engine = m_app.getEngine();

		auto* testModel   = engine.createModelResource("testModel", "resources/models/cube.obj");
		auto* testTexture = engine.createTextureResource("testTexture", "resources/textures/cube.jpg");

		erb::RenderObject object;

		object.transform.setPosition(glm::vec3(-2.0F, 0.0F, -1.0F));
		object.transform.setRotation(glm::vec3(0.0F, 0.0F, 0.0F));
		object.transform.setScale(glm::vec3(0.5F, 0.5F, 0.5F));

		object.materials.emplace(testModel, testTexture);
		addEntity(object);
	}

	void Game::onUpdate()
	{
		destroyOldEntities();
	}

	void Game::onDraw()
	{
	}

	void Game::onResize(const int width, const int height)
	{
	}

	void Game::onCleanup()
	{
	}

	void Game::cleanup()
	{
		m_app.cleanup();
	}

	std::pair<std::int32_t, erb::RenderObject*> Game::getRenderObjectIndex(erb::RenderObject& object)
	{
		auto target = std::ranges::find_if(m_gameObjects,
		    [&object](erb::RenderObject& candidate)
		    {
			    return &candidate == &object;
		    });

		if (target == m_gameObjects.end())
			return {-1, target.base()};

		auto index = std::distance(m_gameObjects.begin(), target);
		return {static_cast<std::int32_t>(index), target.base()};
	}

	void Game::removeEntity(erb::RenderObject& object)
	{
		auto [index, target] = getRenderObjectIndex(object);
		if (index == -1)
			return;

		auto& engine = m_app.getEngine();
		engine.destroyEntity(*target);
	}

	void Game::addEntity(erb::RenderObject& object)
	{
		m_gameObjects.push_back(std::move(object));
		auto& gameObject = m_gameObjects.at(m_gameObjects.size() - 1);

		auto& engine = m_app.getEngine();
		engine.createEntity(&gameObject);
	}

	void Game::destroyOldEntities()
	{
		for (auto& object : m_gameObjects)
		{
			auto [index, renderObject] = getRenderObjectIndex(object);
			if (object.hasBeenDestroyed)
				m_gameObjects.erase(m_gameObjects.begin() + index);
		}
	}
} // namespace fd
#include "erebus/utils/ModelLoader.h"
#include "erebus/utils/Texture.h"
#include <application/Game.h>
#include <glm/ext/vector_float3.hpp>

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

		m_object.transform.setPosition(glm::vec3(-2.0F, 0.0F, -1.0F));
		m_object.transform.setRotation(glm::vec3(0.0F, 0.0F, 0.0F));
		m_object.transform.setScale(glm::vec3(0.5F, 0.5F, 0.5F));

		m_object.materials.emplace(testModel, testTexture);
		engine.createEntity(&m_object);

		m_object1.transform.setPosition(glm::vec3(2.0F, 0.0F, 1.0F));
		m_object1.transform.setRotation(glm::vec3(0.0F, 0.0F, 0.0F));
		m_object1.transform.setScale(glm::vec3(0.5F, 0.5F, 0.5F));

		m_object1.materials.emplace(testModel, testTexture);
		engine.createEntity(&m_object1);
	}

	void Game::onUpdate()
	{
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
} // namespace fd
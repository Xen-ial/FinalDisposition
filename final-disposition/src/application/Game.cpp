#include <application/Game.h>

#include <erebus/engine/Camera.h>
#include <erebus/engine/Input.h>
#include <erebus/vulkan/RenderObject.h>
#include <erebus/utils/PhysicsManager.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/constants.hpp>

#include <GLFW/glfw3.h>

#include <cstdlib>

#ifdef FINAL_DISPOSITION_EDITOR_MODE
constexpr bool EDITOR_MODE = true;
#else
constexpr bool EDITOR_MODE = false;
#endif

namespace fd
{
#pragma region Initialization

	Game::Game()
	{
		m_app.init("Final Disposition", 800, 600);

		m_app.setOnInitCallback([this]()
		    { onInit(); });

		m_app.setOnUpdateCallback([this]()
		    { onUpdate(); });

		m_app.setOnFixedUpdateCallback([this]()
		    { onFixedUpdate(); });

		m_app.setOnDrawCallback([this]()
		    { onDraw(); });

		m_app.setOnResizeCallback([this](int width, int height)
		    { onResize(width, height); });

		m_app.setOnCleanupCallback([this]()
		    { onCleanup(); });
	}

	void Game::initResources()
	{
		auto& input   = m_app.getInput();
		auto& engine  = m_app.getEngine();
		auto& physics = engine.getPhysicsManager();

		auto* horrorRoomModel   = engine.createModelResource("HorrorRoom", "resources/models/HorrorRoom.obj");
		auto* horrorRoomTexture = engine.createTextureResource("HorrorRoomTexture", "resources/textures/RoomAtlas.png");

		erb::RenderObject horrorRoom("HorrorRoom");

		horrorRoom.setModel(*horrorRoomModel);
		horrorRoom.setTexture(*horrorRoomTexture);

		physics.createPhysicsBody(horrorRoom, erb::PhysicsObjectType::STATIC_OBJECT);
		erb::PhysicsManager::createMeshShape(horrorRoom);

		engine.addEntity(horrorRoom);

		// Test model for physics (DELETE ME LATER)
		auto* testModel   = engine.createModelResource("testModel", "resources/models/cube.obj");
		auto* testTexture = engine.createTextureResource("testTexture", "resources/textures/cube.jpg");

		erb::RenderObject object2;
		object2.objName = "box";

		object2.transform.setPosition(glm::vec3(-4.0F, 8.5F, 0.0F));
		object2.transform.setRotation(glm::vec3(0.0F, 0.0F, 0.0F));
		object2.transform.setScale(glm::vec3(1.0F, 1.0F, 1.0F));

		object2.setModel(*testModel);
		object2.setTexture(*testTexture);

		physics.createPhysicsBody(object2, erb::PhysicsObjectType::DYNAMIC_OBJECT);
		erb::PhysicsManager::createBoxHullShape(object2, object2.transform.getScale(), {});

		engine.addEntity(object2);
	}

	void Game::onInit()
	{
		auto& input   = m_app.getInput();
		auto& engine  = m_app.getEngine();
		auto& physics = engine.getPhysicsManager();

		physics.setArePhysicsUpdatesEnabled(true);
		input.setCursorMode(erb::CursorModes::CURSOR_DISABLED);

		if (!EDITOR_MODE)
		{
			m_playerCharacter = Player(engine);
			m_playerCharacter.init();
			m_playerCharacter.setPlayerPosition(glm::vec3(-4.0F, 8.5F, 0.0F));
		}

		initResources();
	}

#pragma endregion

#pragma region GameLoop

	void Game::run()
	{
		m_app.run();
	}

	void Game::onFixedUpdate()
	{
		auto&       engine    = m_app.getEngine();
		const float deltaTime = engine.deltaTime();

		if (!EDITOR_MODE)
		{
			m_playerCharacter.fixedUpdate(deltaTime);
		}
	}

	void Game::onUpdate()
	{
		auto& input  = m_app.getInput();
		auto& engine = m_app.getEngine();
		auto& camera = engine.getCamera();

		const float deltaTime = engine.deltaTime();

		if (input.isKeyPressed(GLFW_KEY_F1))
		{
			m_hasFocus = !m_hasFocus;

			if (m_hasFocus)
				input.setCursorMode(erb::CursorModes::CURSOR_DISABLED);
			else
				input.setCursorMode(erb::CursorModes::DEFAULT);
		}

		if (input.isKeyPressed(GLFW_KEY_ESCAPE))
			std::exit(EXIT_SUCCESS);

		if (!EDITOR_MODE)
		{
			m_playerCharacter.update(deltaTime);
		}

		if (EDITOR_MODE)
		{
			if (input.isKeyDown(GLFW_KEY_W))
				camera.processKeyboard(erb::CameraMovement::FORWARD, deltaTime);

			if (input.isKeyDown(GLFW_KEY_S))
				camera.processKeyboard(erb::CameraMovement::BACKWARD, deltaTime);

			if (input.isKeyDown(GLFW_KEY_D))
				camera.processKeyboard(erb::CameraMovement::RIGHT, deltaTime);

			if (input.isKeyDown(GLFW_KEY_A))
				camera.processKeyboard(erb::CameraMovement::LEFT, deltaTime);

			if (m_hasFocus)
			{
				const auto& mousePos = input.getRelativeMousePosition();
				camera.processMouseMovement(mousePos.x, mousePos.y);
			}
		}
	}

	void Game::onDraw()
	{
	}

	void Game::onResize(const int width, const int height)
	{
	}

#pragma endregion

#pragma region CleanUp

	void Game::onCleanup()
	{
	}

	void Game::cleanup()
	{
		m_app.cleanup();
	}

#pragma endregion
} // namespace fd
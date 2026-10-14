#pragma once
#include "erebus/vulkan/UniformBuffer.h"
#include <erebus/vulkan/Renderer.h>
#include <erebus/vulkan/RenderObject.h>

#include <erebus/engine/Window.h>
#include <erebus/engine/Input.h>
#include <erebus/engine/Camera.h>

#include <erebus/utils/Model.h>
#include <erebus/utils/Texture.h>
#include <erebus/utils/Material.h>
#include <erebus/utils/ResourceManager.h>
#include <erebus/utils/Transform.h>
#include <erebus/utils/PhysicsManager.h>
#include <erebus/utils/Raycast.h>

#include <functional>
#include <expected>

namespace erb
{
	using FixedUpdateCallback = std::function<void()>;

	class Engine
	{
	public:
		Engine(Window& window)
		    : m_window(window), m_renderer(m_window.get()),
		      m_input(m_window.get()) {}

		bool init(const std::string& applicationName);

		void removeEntity(RenderObject& object);
		void addEntity(RenderObject& object);

		std::expected<RenderObject*, bool> findRenderObject(const std::string& name);
		std::expected<RenderObject*, bool> findRenderObject(std::uint32_t objId);

		void setCamera(Camera3D& camera);

		Model*   createModelResource(const std::string& name, const std::string& path);
		Texture* createTextureResource(const std::string& name, const std::string& path);

		void destroyTextureRes(const std::string& name);
		void destroyModelRes(const std::string& name);

		Model*   getModel(const std::string& name);
		Texture* getTexture(const std::string& name);

		void render();
		void cleanup();

		void setFixedUpdateCallback(const FixedUpdateCallback& callback) { m_fixedUpdateCallback = callback; }

		Renderer&       getRenderer() { return m_renderer; }
		Window&         getWindow() { return m_window; }
		Input&          getInput() { return m_input; }
		Camera3D&       getCamera() { return m_camera; }
		PhysicsManager& getPhysicsManager() { return m_physicsManager; }

		std::vector<RenderObject>& getRenderObjects() { return m_renderObjects; }
		RenderObject&              getNamedRenderObject(const std::string& name);

		float deltaTime() const { return m_renderer.getDeltaTime(); }

		std::vector<PointLight>& getPointLights() { return m_renderer.getPointLights(); }
		std::vector<SpotLight>&  getSpotLights() { return m_renderer.getSpotLights(); }

		DirectionalLight& getDirectionalLight() { return m_renderer.getDirectionalLight(); }

		std::pair<std::int32_t, RenderObject*> getRenderObjectIndex(RenderObject& object);

		const Raycast& getRaycastManager() const { return m_physicsManager.getRaycastManager(); }

	private:
		std::reference_wrapper<Window> m_window;

		std::vector<RenderObject> m_renderObjects;

		Camera3D       m_camera;
		Input          m_input;
		Renderer       m_renderer;
		PhysicsManager m_physicsManager;

		float m_accumulator{};

		FixedUpdateCallback m_fixedUpdateCallback;

		void destroyOldEntities();

		void createEntity(RenderObject* renderObject);
		void destroyEntity(RenderObject& renderObject);

		void fixedUpdate();
	};
} // namespace erb
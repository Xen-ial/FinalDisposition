#include <erebus/engine/Engine.h>

#include <print>

namespace erb
{
	bool Engine::init(const std::string& applicationName)
	{
		if (!m_renderer.init(applicationName, &m_renderObjects))
		{
			std::println("failed to init renderer in engine");
			return false;
		}

		m_input.init();
		m_physicsManager.init(m_renderObjects);

		setCamera(m_defaultCamera);
		return true;
	}

	void Engine::createEntity(RenderObject* renderObject)
	{
		m_renderer.addRenderObject(renderObject);
	}

	void Engine::setCamera(Camera3D& camera)
	{
		m_camera = &camera;
	}

	void Engine::destroyEntity(RenderObject& renderObject)
	{
		renderObject.shouldBeDestroyed = true;
	}

	Model* Engine::createModelResource(const std::string& name, const std::string& path)
	{
		auto& resources = m_renderer.getResources();
		auto  model     = Renderer::createModel(path);
		return resources.addModel(name, model);
	}

	Texture* Engine::createTextureResource(const std::string& name, const std::string& path)
	{
		auto& resources = m_renderer.getResources();
		auto  texture   = m_renderer.createTexture(path);
		return resources.addTexture(name, texture);
	}

	void Engine::destroyTextureRes(const std::string& name)
	{
		auto& resources = m_renderer.getResources();
		resources.removeTexture(name);
	}

	void Engine::destroyModelRes(const std::string& name)
	{
		auto& resources = m_renderer.getResources();
		resources.removeModel(name);
	}

	Model* Engine::getModel(const std::string& name)
	{
		auto& resources = m_renderer.getResources();
		return resources.getModel(name);
	}

	Texture* Engine::getTexture(const std::string& name)
	{
		auto& resources = m_renderer.getResources();
		return resources.getTexture(name);
	}

	void Engine::fixedUpdate()
	{
		PhysicsManager::syncTransformsToPhysics(m_renderObjects);

		m_accumulator += m_renderer.getDeltaTime();
		while (m_accumulator >= m_physicsManager.getFixedTimeStep())
		{
			if (m_fixedUpdateCallback != nullptr)
				m_fixedUpdateCallback();

			m_physicsManager.update(m_renderObjects);
			m_accumulator -= m_physicsManager.getFixedTimeStep();
		}

		PhysicsManager::syncPhysicsToTransforms(m_renderObjects);
	}

	void Engine::render()
	{
		fixedUpdate();

		m_renderer.render(*m_camera);
		m_input.resetFrame();
	}

	void Engine::cleanup()
	{
		m_renderer.cleanup();
		m_physicsManager.cleanup(m_renderObjects);
	}

	std::pair<std::int32_t, erb::RenderObject*> Engine::getRenderObjectIndex(erb::RenderObject& object)
	{
		auto target = std::ranges::find_if(m_renderObjects,
		    [&object](erb::RenderObject& candidate)
		    {
			    return &candidate == &object;
		    });

		if (target == m_renderObjects.end())
			return {-1, target.base()};

		auto index = std::distance(m_renderObjects.begin(), target);
		return {static_cast<std::int32_t>(index), target.base()};
	}

	void Engine::removeEntity(erb::RenderObject& object)
	{
		auto [index, target] = getRenderObjectIndex(object);
		if (index == -1)
			return;

		destroyEntity(*target);
	}

	void Engine::addEntity(erb::RenderObject& object)
	{
		m_renderObjects.push_back(std::move(object));
		auto& gameObject = m_renderObjects.at(m_renderObjects.size() - 1);
		createEntity(&gameObject);
	}

	std::expected<RenderObject*, bool> Engine::findRenderObject(const std::string& name)
	{
		for (auto& object : m_renderObjects)
		{
			if (object.objName == name)
				return &object;
		}

		return std::unexpected(true);
	}

	std::expected<RenderObject*, bool> Engine::findRenderObject(std::uint32_t objId)
	{
		for (auto& object : m_renderObjects)
		{
			if (object.objId == objId)
				return &object;
		}

		return std::unexpected(true);
	}

	void Engine::destroyOldEntities()
	{
		for (auto& object : m_renderObjects)
		{
			auto [index, renderObject] = getRenderObjectIndex(object);
			if (object.hasBeenDestroyed)
			{
				PhysicsManager::destroyPhysicsObject(*renderObject);
				m_renderObjects.erase(m_renderObjects.begin() + index);
			}
		}
	}

	RenderObject& Engine::getNamedRenderObject(const std::string& name)
	{
		for (auto& object : m_renderObjects)
		{
			if (object.objName != name) continue;
			return object;
		}

		return m_renderObjects[m_renderObjects.size() - 1];
	}
} // namespace erb
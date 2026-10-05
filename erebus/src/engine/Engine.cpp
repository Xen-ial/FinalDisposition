#include <erebus/engine/Engine.h>

#include <print>

namespace erb
{
	bool Engine::init(const std::string& applicationName)
	{
		if (!m_renderer.init(applicationName))
		{
			std::println("failed to init renderer in engine");
			return false;
		}

		return true;
	}

	void Engine::createEntity(RenderObject* renderObject)
	{
		m_renderer.addRenderObject(renderObject);
	}

	void Engine::destroyEntity(RenderObject* renderObject)
	{
		m_renderer.removeRenderObject(renderObject);
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

	void Engine::render()
	{
		m_renderer.render();
	}

	void Engine::cleanup()
	{
		m_renderer.cleanup();
	}
} // namespace erb
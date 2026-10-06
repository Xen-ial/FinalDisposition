#pragma once
#include <erebus/vulkan/Renderer.h>
#include <erebus/vulkan/RenderObject.h>

#include <erebus/engine/Window.h>
#include <erebus/engine/Input.h>

#include <erebus/utils/Model.h>
#include <erebus/utils/Texture.h>
#include <erebus/utils/Material.h>
#include <erebus/utils/ResourceManager.h>
#include <erebus/utils/Transform.h>

#include <functional>

namespace erb
{
	class Engine
	{
	public:
		Engine(Window& window)
		    : m_window(window), m_renderer(m_window.get()), m_input(m_window.get()) {}

		bool init(const std::string& applicationName);

		void createEntity(RenderObject* renderObject);
		void destroyEntity(RenderObject& renderObject);

		Model*   createModelResource(const std::string& name, const std::string& path);
		Texture* createTextureResource(const std::string& name, const std::string& path);

		void destroyTextureRes(const std::string& name);
		void destroyModelRes(const std::string& name);

		Model*   getModel(const std::string& name);
		Texture* getTexture(const std::string& name);

		void render();

		void cleanup();

		Renderer& getRenderer() { return m_renderer; }
		Window&   getWindow() { return m_window; }
		Input&    getInput() { return m_input; }

	private:
		std::reference_wrapper<Window> m_window;

		Input    m_input;
		Renderer m_renderer;
	};
} // namespace erb
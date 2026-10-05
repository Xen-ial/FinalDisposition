#include <erebus/engine/App.h>

#include <GLFW/glfw3.h>

#include <print>

namespace erb
{
	bool App::init(const std::string& appName, const int width, const int height)
	{
		m_window.create(width, height, appName);
		m_engine.emplace(m_window);

		if (!m_engine->init(appName))
		{
			std::println("failed to initialize renderer: app({})", appName);
			m_isInitialized = false;
			return false;
		}

		m_isInitialized = true;
		return true;
	}

	void App::run()
	{
		if (!m_isInitialized)
		{
			std::println("app not initialized did you forget to call App::init() ?");
			return;
		}

		if (m_onInit != nullptr)
			m_onInit();

		while (!m_window.shouldClose())
		{
			glfwPollEvents();

			if (m_onUpdate != nullptr)
				m_onUpdate();

			if (m_onDraw != nullptr)
				m_onDraw();

			m_engine->render();

			if (m_window.hasFramebufferResized)
			{
				int width{};
				int height{};

				glfwGetWindowSize(m_window.handle, &width, &height);
				m_engine->getRenderer().setFramebufferSize(width, height);

				if (m_onResize != nullptr)
					m_onResize(width, height);
			}
		}

		cleanup();
	}

	void App::cleanup()
	{
		if (m_onCleanup != nullptr)
			m_onCleanup();

		m_engine->cleanup();
		m_window.destroy();
	}
} // namespace erb
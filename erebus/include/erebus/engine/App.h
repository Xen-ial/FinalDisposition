#pragma once

#include <erebus/engine/Engine.h>
#include <erebus/engine/Window.h>

#include <functional>
#include <optional>

namespace erb
{
	using EventCallback  = std::function<void()>;
	using ResizeCallback = std::function<void(int width, int height)>;

	class App
	{
	public:
		bool init(const std::string& appName, int width, int height);

		void run();
		void cleanup();

		void setOnInitCallback(const EventCallback& callback) { m_onInit = callback; }
		void setOnUpdateCallback(const EventCallback& callback) { m_onUpdate = callback; }
		void setOnDrawCallback(const EventCallback& callback) { m_onDraw = callback; }
		void setOnCleanupCallback(const EventCallback& callback) { m_onCleanup = callback; }

		void setOnResizeCallback(const ResizeCallback& callback) { m_onResize = callback; }

		Window& getWindow() { return m_window; }
		Engine& getEngine() { return m_engine.value(); }
		Input&  getInput() { return m_engine->getInput(); }

	private:
		std::optional<Engine> m_engine;

		Window m_window{};

		bool m_isInitialized = false;

		EventCallback m_onInit;
		EventCallback m_onUpdate;
		EventCallback m_onDraw;
		EventCallback m_onCleanup;

		ResizeCallback m_onResize;
	};
} // namespace erb
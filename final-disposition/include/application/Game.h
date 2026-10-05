#pragma once

#include <erebus/engine/App.h>

namespace fd
{
	class Game
	{
	public:
		Game();

		void run();

	private:
		erb::App m_app;

		erb::RenderObject m_object;
		erb::RenderObject m_object1;

		void onInit();
		void onUpdate();
		void onDraw();
		void onResize(int width, int height);
		void onCleanup();

		void cleanup();
	};
} // namespace fd
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

		bool m_hasFocus{true};

		void onInit();
		void onUpdate();
		void onDraw();
		void onResize(int width, int height);
		void onCleanup();

		void cleanup();
	};
} // namespace fd
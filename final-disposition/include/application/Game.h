#pragma once
#include <erebus/engine/App.h>
#include <character/Player.h>

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

		Player m_playerCharacter;

		void onInit();
		void onUpdate();
		void onFixedUpdate();
		void onDraw();
		void onResize(int width, int height);
		void onCleanup();

		void cleanup();
	};
} // namespace fd
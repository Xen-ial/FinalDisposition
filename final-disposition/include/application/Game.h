#pragma once

#include "erebus/vulkan/RenderObject.h"
#include <erebus/engine/App.h>

#include <vector>

namespace fd
{
	class Game
	{
	public:
		Game();

		void run();

	private:
		erb::App m_app;

		std::vector<erb::RenderObject> m_gameObjects;

		bool m_hasFocus{true};

		void onInit();
		void onUpdate();
		void onDraw();
		void onResize(int width, int height);
		void onCleanup();

		void cleanup();

		void removeEntity(erb::RenderObject& object);
		void addEntity(erb::RenderObject& object);

		void destroyOldEntities();

		std::pair<std::int32_t, erb::RenderObject*> getRenderObjectIndex(erb::RenderObject& object);
	};
} // namespace fd
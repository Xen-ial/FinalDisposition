#pragma once
#include <erebus/engine/Engine.h>
#include <character/FPController.h>

// TODO: Setup interaction with environment inside the first person controller

namespace fd
{
	class Player
	{
	public:
		Player() = default;
		Player(erb::Engine& engine) : m_engine(&engine) {}

		void init();
		void update(float deltaTime);
		void fixedUpdate(float deltaTime);

		FPController& getFPController() { return m_fpController; }

		void setPlayerPosition(glm::vec3 position)
		{
			auto* fpController = m_fpController.getFPRenderObject();
			if (fpController == nullptr)
				return;

			fpController->transform.setPosition(position);
		}

	private:
		FPController m_fpController;

		erb::Engine* m_engine = nullptr;
	};
} // namespace fd
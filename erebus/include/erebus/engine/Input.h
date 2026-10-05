#pragma once

#include "erebus/engine/Window.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <GLFW/glfw3.h>

#include <functional>

// TODO: Implement input handling (mouse & keyboard)

namespace erb
{
	class Input
	{
	public:
		Input(Window& window)
		    : m_window(window) {}

	private:
		std::reference_wrapper<Window> m_window;

		glm::vec2 m_mousePosition{glm::vec2(0.0F)};
	};
} // namespace erb
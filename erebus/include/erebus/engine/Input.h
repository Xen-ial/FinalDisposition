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
#include <cstdint>
#include <unordered_set>

namespace erb
{
	enum class ButtonState : std::uint8_t
	{
		DOWN = 0,
		UP
	};

	enum class CursorModes : std::uint8_t
	{
		DEFAULT = 0,
		CURSOR_HIDDEN,
		CURSOR_CAPTURED,
		CURSOR_DISABLED
	};

	class Input
	{
	public:
		Input(Window& window)
		    : m_window(window) {}

		void init();
		void resetFrame();

		bool isMouseDown(int button);
		bool isMouseButtonPressed(int button);
		bool isMouseButtonReleased(int button);

		bool isKeyDown(int key);
		bool isKeyPressed(int key);
		bool isKeyReleased(int key);

		void setCursorMode(const CursorModes& cursorMode);

		[[nodiscard]] const glm::vec2& getAbsoluteMousePosition() const { return m_mouseAbsolutePosition; }
		[[nodiscard]] const glm::vec2& getRelativeMousePosition() const { return m_mouseRelativePosition; }
		[[nodiscard]] const glm::vec2& getScrollOffset() const { return m_mouseScrollOffset; }

	private:
		std::reference_wrapper<Window> m_window;

		glm::vec2 m_mouseAbsolutePosition{glm::vec2(0.0F, 0.0F)};
		glm::vec2 m_mouseRelativePosition{glm::vec2(0.0F, 0.0F)};
		glm::vec2 m_mouseLastPosition{glm::vec2(0.0F, 0.0F)};
		glm::vec2 m_mouseScrollOffset{glm::vec2(0.0F, 0.0F)};

		bool m_hadFirstMouseMovement{false};

		std::unordered_set<int> m_keysDown;
		std::unordered_set<int> m_keysPressed;
		std::unordered_set<int> m_keysReleased;

		std::unordered_set<int> m_mouseButtonsDown;
		std::unordered_set<int> m_mouseButtonsPresssed;
		std::unordered_set<int> m_mouseButtonsReleased;

		void processKeyboardEvents(int key, const ButtonState& state);
		void processMouseEvents(int button, const ButtonState& state);

		void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
		void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	};
} // namespace erb
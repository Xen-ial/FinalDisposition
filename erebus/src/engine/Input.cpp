#include <erebus/engine/Input.h>

#include <GLFW/glfw3.h>

#include <print>
#include <variant>

namespace erb
{
	void Input::keyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
	{
		switch (action)
		{
		case GLFW_PRESS:
			processKeyboardEvents(key, ButtonState::DOWN);
			break;
		case GLFW_RELEASE:
			processKeyboardEvents(key, ButtonState::UP);
			break;
		case GLFW_REPEAT:
			processKeyboardEvents(key, ButtonState::DOWN);
			break;
		default:
			break;
		}
	}

	void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		switch (action)
		{
		case GLFW_PRESS:
			processMouseEvents(button, ButtonState::DOWN);
			break;
		case GLFW_RELEASE:
			processMouseEvents(button, ButtonState::UP);
			break;
		case GLFW_REPEAT:
			processMouseEvents(button, ButtonState::DOWN);
			break;
		default:
			break;
		}
	}

	void Input::cursorPosCallback(GLFWwindow* window, const double xPos, const double yPos)
	{
		const glm::vec2 mousePos{xPos, yPos};

		m_mouseAbsolutePosition = mousePos;

		if (!m_hadFirstMouseMovement)
		{
			m_mouseLastPosition     = mousePos;
			m_hadFirstMouseMovement = true;
		}

		const float xOffset = mousePos.x - m_mouseLastPosition.x;
		const float yOffset = m_mouseLastPosition.y - mousePos.y;

		m_mouseLastPosition     = mousePos;
		m_mouseRelativePosition = glm::vec2{xOffset, yOffset};
	}

	void Input::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		glm::vec2 scrollOffset{xOffset, yOffset};
		m_mouseScrollOffset = scrollOffset;
	}

	void Input::init()
	{
		auto& glfwWindow = m_window.get().handle;

		glfwSetWindowUserPointer(glfwWindow, this);
		glfwSetKeyCallback(glfwWindow,
		    [](GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
		    {
			    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
			    if (input != nullptr)
				    input->keyCallback(window, key, scancode, action, mods);
		    });

		glfwSetMouseButtonCallback(glfwWindow,
		    [](GLFWwindow* window, const int button, const int action, const int mods)
		    {
			    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
			    if (input != nullptr)
				    input->mouseButtonCallback(window, button, action, mods);
		    });

		glfwSetCursorPosCallback(glfwWindow,
		    [](GLFWwindow* window, const double xPos, const double yPos)
		    {
			    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
			    if (input != nullptr)
				    input->cursorPosCallback(window, xPos, yPos);
		    });

		glfwSetScrollCallback(glfwWindow,
		    [](GLFWwindow* window, const double xOffset, const double yOffset)
		    {
			    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
			    if (input != nullptr)
				    input->scrollCallback(window, xOffset, yOffset);
		    });

		resetFrame();
	}

	void Input::resetFrame()
	{
		m_keysPressed.clear();
		m_keysReleased.clear();

		m_mouseButtonsPresssed.clear();
		m_mouseButtonsReleased.clear();

		m_mouseRelativePosition = {};
	}

	bool Input::isKeyDown(int key)
	{
		return m_keysDown.contains(key);
	}

	bool Input::isKeyPressed(int key)
	{
		return m_keysPressed.contains(key);
	}

	bool Input::isKeyReleased(int key)
	{
		return m_keysReleased.contains(key);
	}

	void Input::processKeyboardEvents(int key, const ButtonState& state)
	{
		switch (state)
		{
		case ButtonState::DOWN:
			if (m_keysDown.insert(key).second)
				m_keysPressed.insert(key);
			break;
		case ButtonState::UP:
			if (m_keysDown.erase(key) > 0)
				m_keysReleased.insert(key);
			break;
		default:
			std::println("keyboard state not supported");
			break;
		}
	}

	bool Input::isMouseDown(int button)
	{
		return m_mouseButtonsDown.contains(button);
	}

	bool Input::isMouseButtonPressed(int button)
	{
		return m_mouseButtonsPresssed.contains(button);
	}

	bool Input::isMouseButtonReleased(int button)
	{
		return m_mouseButtonsReleased.contains(button);
	}

	void Input::processMouseEvents(int button, const ButtonState& state)
	{
		switch (state)
		{
		case ButtonState::DOWN:
			if (m_mouseButtonsDown.insert(button).second)
				m_mouseButtonsPresssed.insert(button);
			break;
		case ButtonState::UP:
			if (m_mouseButtonsDown.erase(button) > 0)
				m_mouseButtonsReleased.insert(button);
			break;
		default:
			std::println("mouse state not suppported");
			break;
		}
	}

	void Input::setCursorMode(const CursorModes& cursorMode)
	{
		auto& glfwWindow = m_window.get().handle;

		switch (cursorMode)
		{
		case CursorModes::DEFAULT:
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case CursorModes::CURSOR_HIDDEN:
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
		case CursorModes::CURSOR_CAPTURED:
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
			break;
		case CursorModes::CURSOR_DISABLED:
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		}
	}
} // namespace erb
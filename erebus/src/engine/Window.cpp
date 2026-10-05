#include <erebus/engine/Window.h>

#include <print>

namespace erb
{
	void Window::create(const int width, const int height, const std::string& title)
	{
		if (glfwInit() == GLFW_FALSE)
		{
			std::println("failed to init glfw!");
			return;
		}

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_FALSE);

		handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		if (handle == nullptr)
		{
			std::println("failed to create glfw window!");
			return;
		}

		glfwSetWindowUserPointer(handle, this);
		glfwSetFramebufferSizeCallback(handle, Window::framebufferCallback);
	}

	void Window::framebufferCallback(GLFWwindow* window, int width, int height)
	{
		auto* app = static_cast<Window*>(glfwGetWindowUserPointer(window));

		app->hasFramebufferResized = true;

		app->width  = width;
		app->height = height;
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(handle) == GLFW_TRUE;
	}

	void Window::destroy() const
	{
		glfwDestroyWindow(handle);
		glfwTerminate();
	}
} // namespace erb
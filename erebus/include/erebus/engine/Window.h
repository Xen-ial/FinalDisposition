#pragma once

#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace erb
{
	constexpr std::uint32_t DEFAULT_WINDOW_WIDTH  = 800;
	constexpr std::uint32_t DEFAULT_WINDOW_HEIGHT = 600;

	struct Window
	{
		GLFWwindow* handle                = nullptr;
		bool        hasFramebufferResized = false;

		int width{};
		int height{};

		void create(int width, int height, const std::string& title);

		[[nodiscard]] bool shouldClose() const;

		static void framebufferCallback(GLFWwindow* window, int width, int height);

		void destroy() const;
	};
} // namespace erb
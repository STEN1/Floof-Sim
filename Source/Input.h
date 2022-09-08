#pragma once
#include <GLFW/glfw3.h>
#include "Floof.h"
#include "Math.h"
#include "Logger.h"

namespace FLOOF {
	class Input {
	public:
		static int Key(int keyCode) {
			return glfwGetKey(s_Window, keyCode);
		}
		static int MouseButton(int button) {
			return glfwGetMouseButton(s_Window, button);
		}
		static glm::vec2 MousePos() {
			double x, y;
			glfwGetCursorPos(s_Window, &x, &y);
			return glm::vec2(x, y);
		}
		inline static GLFWwindow* s_Window = nullptr;
	};
}
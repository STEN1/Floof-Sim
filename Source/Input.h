#pragma once
#include <glfw/glfw3.h>
#include "Floof.h"

namespace FLOOF {
	class Input {
	public:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
			LOG("Key pressed: {} with action: {}\n", scancode, action);
			Keys[key] = action;
		}
		inline static std::unordered_map<int, bool> Keys;
	};
}
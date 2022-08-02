#pragma once
#include "VulkanRenderer.h"

#include <entt/entt.hpp>

namespace FLOOF {
	class Application {
	public:
		Application();
		~Application();
		int Run();
	private:
		void Update(double deltaTime);
		void Draw();
		entt::registry m_Registry;
		GLFWwindow* m_Window;
		VulkanRenderer* m_Renderer;
	};
}
#include "Application.h"
#include "Timer.h"

#include <string>

namespace FLOOF {
	Application::Application() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
		m_Renderer = new VulkanRenderer(m_Window);
	}

	Application::~Application() {
		delete m_Renderer;
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	int Application::Run() {
		Timer timer;
		while (!glfwWindowShouldClose(m_Window)) {
			glfwPollEvents();
			double deltaTime = timer.Delta();
			double fps{};
			if (deltaTime > 0.0) {
				fps = 1.0 / deltaTime;
			}
			std::string title = "FPS: " + std::to_string(fps);
			glfwSetWindowTitle(m_Window, title.c_str());
			m_Renderer->Draw();
		}
		m_Renderer->Finish();
		return 0;
	}
}
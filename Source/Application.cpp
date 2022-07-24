#include "Application.h"

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
		while (!glfwWindowShouldClose(m_Window)) {
			glfwPollEvents();
		}
		return 0;
	}
}
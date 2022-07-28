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
		float titleBarUpdateTimer{};
		float titlebarUpdateRate = 0.1f;
		float frameCounter{};
		while (!glfwWindowShouldClose(m_Window)) {
			glfwPollEvents();
			double deltaTime = timer.Delta();
			frameCounter++;
			titleBarUpdateTimer += deltaTime;
			if (titleBarUpdateTimer > titlebarUpdateRate) {
				float avgDeltaTime = titleBarUpdateTimer / frameCounter;
				float fps{};
				fps = 1.0 / avgDeltaTime;
				std::string title = "FPS: " + std::to_string(fps);
				glfwSetWindowTitle(m_Window, title.c_str());
				titleBarUpdateTimer = 0.f;
				frameCounter = 0.f;
			}
			Update(deltaTime);
			m_Renderer->Draw();
		}
		m_Renderer->Finish();
		return 0;
	}
	void Application::Update(double deltaTime) {

	}
}
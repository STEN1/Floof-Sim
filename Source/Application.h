#pragma once
#include "VulkanRenderer.h"

namespace FLOOF {
class Application {
public:
	Application();
	~Application();
	int Run();
private:
	GLFWwindow* m_Window;
	VulkanRenderer* m_Renderer;
};
}
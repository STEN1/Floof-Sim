#pragma once
#include "VulkanRenderer.h"

#include <entt/entt.hpp>

#include <unordered_map>
#include "Logger.h"
#include <memory>

namespace FLOOF {
	class Application {
	public:
		Application();
		~Application();
		int Run();
	private:
		void Update(double deltaTime);
		void Simulate(double deltaTime);
		void Draw();
		entt::registry m_Registry;
		GLFWwindow* m_Window;
		VulkanRenderer* m_Renderer;
		entt::entity m_CameraEntity;
	};
}
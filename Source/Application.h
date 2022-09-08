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
       std::shared_ptr<Utils::Logger> logger;
	private:
		void Update(double deltaTime);
		void Simulate(double deltaTime);
		void Draw();
		entt::registry m_Registry;
		GLFWwindow* m_Window;
		VulkanRenderer* m_Renderer;
		entt::entity m_CameraEntity;
		entt::entity m_TerrainEntity;
	};
}
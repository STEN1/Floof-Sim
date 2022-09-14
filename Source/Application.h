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
		entt::entity m_TerrainEntity;

		// ------------Debug utils----------------
		void DebugInit();
		void DebugClearLineBuffer();
		// Draws line in world coords
		void DebugDrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3 color);
		// Draws triangle on world coords
		void DebugDrawTriangle(const Triangle& triangle, const glm::vec3& color);
		std::vector<LineVertex> m_DebugLineBuffer;
		entt::entity m_DebugLineEntity;
	};
}
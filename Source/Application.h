#pragma once
#include "VulkanRenderer.h"

#include <entt/entt.hpp>

#include <unordered_map>
#include "Logger.h"
#include <memory>
#include <chrono>

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
		ImGuiContext* m_ImguiContext;
		VulkanRenderer* m_Renderer;
		entt::entity m_CameraEntity;
		entt::entity m_TerrainEntity;

        // ----------- Physics utils -------------
        void ResetBall();
        void SpawnBall();
        std::chrono::high_resolution_clock::time_point m_Ballspawntime;

		// ----------- Debug utils ---------------
		void DebugInit();
		void DebugClearLineBuffer();
		void DebugUpdateLineBuffer();
		void DebugToggle();
		void DebugToggleDrawNormals();

		// Draws line in world coords
		void DebugDrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3 color);

		// Draws triangle on world coords
		void DebugDrawTriangle(const Triangle& triangle, const glm::vec3& color);
		std::vector<LineVertex> m_DebugLineBuffer;
		entt::entity m_DebugLineEntity;

		bool m_DebugDraw = true;
		bool m_DrawNormals = false;
		bool m_ShowImguiDemo = true;
	};
}
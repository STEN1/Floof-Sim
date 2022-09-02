#include "Application.h"

#include "Timer.h"
#include "Components.h"

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
		const auto TreeEntity = m_Registry.create();
		m_Registry.emplace<TransformComponent>(TreeEntity);
		m_Registry.emplace<MeshComponent>(TreeEntity, "Assets/HappyTree.obj");
		m_Registry.emplace<TextureComponent>(TreeEntity, "Assets/HappyTree.png");

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
			Simulate(deltaTime);
			Draw();
		}
		m_Renderer->FinishAllFrames();
		m_Registry.clear();
		return 0;
	}
	void Application::Update(double deltaTime) {
		auto view = m_Registry.view<TransformComponent>();
		for (auto [entiry, transform] : view.each()) {
			transform.Rotation.y += deltaTime;
		}
	}
	void Application::Simulate(double deltaTime) {

	}
	void Application::Draw() {
		uint32_t imageIndex = m_Renderer->GetNextSwapchainImage();
		auto commandBuffer = m_Renderer->StartRecording(imageIndex);

		// camera
		auto extent = m_Renderer->GetExtent();
		glm::vec3 camPos = { 0.f, 5.f, -10.f };
		glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
		glm::mat4 projection = glm::perspective<float>(glm::radians(70.f),
			extent.width / (float)extent.height, 0.1f, 1000.f);
		glm::mat4 vp = projection * view;

		// Geometry pass
		auto geoPassView = m_Registry.view<TransformComponent, MeshComponent, TextureComponent>();
		for (auto [entity, transform, mesh, texture] : geoPassView.each()) {
			MeshPushConstants constants;
			constants.mvp = vp * transform.GetTransform();
			vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(MeshPushConstants), &constants);
			texture.Bind(commandBuffer);
			mesh.Draw(commandBuffer);
		}

		m_Renderer->EndRecording();
		m_Renderer->SubmitAndPresent(imageIndex);
	}
}
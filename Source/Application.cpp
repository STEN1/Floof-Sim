#include "Application.h"

#include "Timer.h"
#include "Components.h"
#include "Input.h"

#include <string>

namespace FLOOF {
	Application::Application() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
		m_Renderer = new VulkanRenderer(m_Window);

		glfwSetKeyCallback(m_Window, &Input::KeyCallback);
	}

	Application::~Application() {
		delete m_Renderer;
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	int Application::Run() {
		const auto treeEntity = m_Registry.create();
		m_Registry.emplace<TransformComponent>(treeEntity);
		m_Registry.emplace<MeshComponent>(treeEntity, "Assets/HappyTree.obj");
		m_Registry.emplace<TextureComponent>(treeEntity, "Assets/HappyTree.png");

		m_CameraEntity = m_Registry.create();
		m_Registry.emplace<CameraComponent>(m_CameraEntity);
		m_Registry.emplace<TransformComponent>(m_CameraEntity);
		auto& cameraTransform = m_Registry.get<TransformComponent>(m_CameraEntity);
		cameraTransform.Position = glm::vec3( 0.f, 5.f, -10.f);

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
		{	// Rotate all meshes.
			auto view = m_Registry.view<TransformComponent, MeshComponent>();
			for (auto [entiry, transform, mesh] : view.each()) {
				transform.Rotation.y += deltaTime;
			}
		}
		{	// Update camera.
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto [entity, transform] : view.each()) {
				glm::vec3 dir{};
				if (Input::Keys[GLFW_KEY_W] == GLFW_PRESS) {
					dir.z += 1.f;
				}
				if (Input::Keys[GLFW_KEY_S] == GLFW_PRESS) {
					dir.z -= 1.f;
				}
				if (Input::Keys[GLFW_KEY_D] == GLFW_PRESS) {
					dir.x -= 1.f;
				}
				if (Input::Keys[GLFW_KEY_A] == GLFW_PRESS) {
					dir.x += 1.f;
				}
				if (Input::Keys[GLFW_KEY_E] == GLFW_PRESS) {
					dir.y += 1.f;
				}
				if (Input::Keys[GLFW_KEY_Q] == GLFW_PRESS) {
					dir.y -= 1.f;
				}
				if (dir.z != 0.f || dir.x != 0.f || dir.y != 0.f) {
					static constexpr float speed = 10.f;
					transform.Position += glm::normalize(dir) * speed * (float)deltaTime;
				}
			}
		}
	}
	void Application::Simulate(double deltaTime) {

	}
	void Application::Draw() {
		auto* renderer = VulkanRenderer::Get();
		auto commandBuffer = m_Renderer->StartRecording();

		// Camera setup
		auto extent = m_Renderer->GetExtent();
		auto& cameraTransform = m_Registry.get<TransformComponent>(m_CameraEntity);
		glm::mat4 view = glm::translate(glm::mat4(1.f), cameraTransform.Position);
		glm::mat4 projection = glm::perspective<float>(glm::radians(70.f),
			extent.width / (float)extent.height, 0.1f, 1000.f);
		glm::mat4 vp = projection * view;

		{	// Geometry pass
			renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Basic);
			auto view = m_Registry.view<TransformComponent, MeshComponent, TextureComponent>();
			for (auto [entity, transform, mesh, texture] : view.each()) {
				MeshPushConstants constants;
				constants.mvp = vp * transform.GetTransform();
				vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(MeshPushConstants), &constants);
				texture.Bind(commandBuffer);
				mesh.Draw(commandBuffer);
			}
		}

		m_Renderer->EndRecording();
		m_Renderer->SubmitAndPresent();
	}
}
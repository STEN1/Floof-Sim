#include "Application.h"
#include "LoggerMacros.h"
#include "Timer.h"
#include "Components.h"
#include "Input.h"
#include "Utils.h"

#include <string>

namespace FLOOF {
	Application::Application() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(1600, 900, "Vulkan window", nullptr, nullptr);
		m_Renderer = new VulkanRenderer(m_Window);
		Input::s_Window = m_Window;
        Utils::Logger::s_Logger = new Utils::Logger("Floof.log");

        LOG_INFO("Test of logging");
        LOG_WARNING("Test of logging");
        LOG_ERROR("Test of logging");
        LOG_CRITICAL("Test of logging");

	}

	Application::~Application() {
		delete m_Renderer;

        delete Utils::Logger::s_Logger;

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	int Application::Run() {
		{
			m_TerrainEntity = m_Registry.create();
			m_Registry.emplace<TransformComponent>(m_TerrainEntity);
			auto vertexData = Utils::GetVisimVertexData("Assets/SimTerrain.visim");
			m_Registry.emplace<TerrainComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<MeshComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<TextureComponent>(m_TerrainEntity, "Assets/HappyTree.png");
		}
        {
            const auto entity = m_Registry.create();
            m_Registry.emplace<TransformComponent>(entity);

            //line mesh componenet
            std::vector<LineVertex> lines;
            LineVertex line;
            line.Pos = glm::vec3();
            lines.push_back(line);
            line.Pos = glm::vec3(0.0,100.0,0.0);
            lines.push_back(line);
            m_Registry.emplace<LineMeshComponent>(entity, lines);
        }
		{
			const auto treeEntity = m_Registry.create();
			m_Registry.emplace<TransformComponent>(treeEntity);
			m_Registry.emplace<MeshComponent>(treeEntity, "Assets/HappyTree.obj");
			m_Registry.emplace<TextureComponent>(treeEntity, "Assets/HappyTree.png");
		}

		{
			const auto treeEntity = m_Registry.create();
			auto& transform = m_Registry.emplace<TransformComponent>(treeEntity);
			m_Registry.emplace<MeshComponent>(treeEntity, "Assets/HappyTree.obj");
			m_Registry.emplace<TextureComponent>(treeEntity, "Assets/HappyTree.png");
			transform.Position.x += 6.f;
			transform.Position.y += 10.f;
		}

		{
			const auto treeEntity = m_Registry.create();
			auto& transform = m_Registry.emplace<TransformComponent>(treeEntity);
			m_Registry.emplace<MeshComponent>(treeEntity, "Assets/HappyTree.obj");
			m_Registry.emplace<TextureComponent>(treeEntity, "Assets/HappyTree.png");
			transform.Position.x -= 6.f;
			transform.Position.y += 10.f;
		}

		{
			m_CameraEntity = m_Registry.create();
			glm::vec3 cameraPos(0.f, 15.f, -40.f);
			m_Registry.emplace<CameraComponent>(m_CameraEntity, cameraPos);
		}

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
		{	// Rotate first mesh.
			auto view = m_Registry.view<TransformComponent, MeshComponent>();
			for (auto [entiry, transform, mesh] : view.each()) {
				transform.Rotation.y += deltaTime;
				break;
			}
		}
		{	// Update camera.
			auto view = m_Registry.view<CameraComponent>();
			for (auto [entity, camera] : view.each()) {
				static constexpr float speed = 10.f;
				float moveAmount = speed * deltaTime;
				if (Input::Key(GLFW_KEY_W) == GLFW_PRESS) {
					camera.MoveForward(moveAmount);
				}
				if (Input::Key(GLFW_KEY_S) == GLFW_PRESS) {
					camera.MoveForward(-moveAmount);
				}
				if (Input::Key(GLFW_KEY_D) == GLFW_PRESS) {
					camera.MoveRight(moveAmount);
				}
				if (Input::Key(GLFW_KEY_A) == GLFW_PRESS) {
					camera.MoveRight(-moveAmount);
				}
				static glm::vec2 oldMousePos = glm::vec2(0.f);
				glm::vec2 mousePos = Input::MousePos();
				glm::vec2 mouseDelta = mousePos - oldMousePos;
				oldMousePos = mousePos;
				static constexpr float mouseSpeed = 0.002f;
				if (Input::MouseButton(GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
					camera.Yaw(mouseDelta.x * mouseSpeed);
					camera.Pitch(mouseDelta.y * mouseSpeed);
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
		CameraComponent& camera = m_Registry.get<CameraComponent>(m_CameraEntity);
		glm::mat4 vp = camera.GetVP(glm::radians(70.f), extent.width / (float)extent.height, 0.1f, 1000.f);

		{	// Geometry pass
			renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Basic);
			auto view = m_Registry.view<TransformComponent, MeshComponent, TextureComponent>();
			for (auto [entity, transform, mesh, texture] : view.each()) {
				MeshPushConstants constants;
				constants.MVP = vp * transform.GetTransform();
                constants.InvModelMat = glm::inverse(transform.GetTransform());
				vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(MeshPushConstants), &constants);
				texture.Bind(commandBuffer);
				mesh.Draw(commandBuffer);
			}
            {
                m_Renderer->BindGraphicsPipeline(commandBuffer,RenderPipelineKeys::Line);
                auto view = m_Registry.view<TransformComponent, LineMeshComponent>();
                for (auto [entity, transform,lineMesh] : view.each()) {
                    LinePushConstants constants;
                    constants.MVP = vp * transform.GetTransform();
                    constants.Color = glm::vec4(0.0,1.0,0.0,1.0);
                    vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
                                       0, sizeof(LinePushConstants), &constants);
                    lineMesh.Draw(commandBuffer);
                }
            }
		}

		m_Renderer->EndRecording();
		m_Renderer->SubmitAndPresent();
	}
}
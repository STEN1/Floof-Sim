#include "Application.h"
#include "LoggerMacros.h"
#include "Timer.h"
#include "Components.h"
#include "Input.h"
#include "Utils.h"
#include "Physics/Physics.h"
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
			auto& terrainTransform = m_Registry.emplace<TransformComponent>(m_TerrainEntity);
			auto vertexData = Utils::GetVisimVertexData("Assets/SimTerrain.visim");
			auto& terrain = m_Registry.emplace<TerrainComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<MeshComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<TextureComponent>(m_TerrainEntity, "Assets/HappyTree.png");

			for (auto& tri : terrain.Triangles) {
				const auto entity = m_Registry.create();
				auto& transform = m_Registry.emplace<TransformComponent>(entity);
				transform = terrainTransform;
				std::vector<LineVertex> line(2);
				LineVertex v;
				v.Pos = (tri.A + tri.B + tri.C) / 3.f;
				line[0] = v;
				v.Pos += tri.N * 5.f;
				line[1] = v;
				auto& mesh = m_Registry.emplace<LineMeshComponent>(entity, line);
				mesh.Color = glm::vec4(1.f, 0.f, 1.f, 1.f);
			}
		}
        {
            const auto entity = m_Registry.create();
            m_Registry.emplace<TransformComponent>(entity);
            std::vector<LineVertex> lines;
            LineVertex line;
            line.Pos = glm::vec3();
            lines.push_back(line);
            line.Pos = glm::vec3(0.f, 200.f, 0.f);
            lines.push_back(line);
            auto& lineMeshComponent = m_Registry.emplace<LineMeshComponent>(entity, lines);
			lineMeshComponent.Color = glm::vec4(0.f, 1.f, 0.f, 1.f);
        }
		{
			const auto entity = m_Registry.create();
			m_Registry.emplace<TransformComponent>(entity);
			std::vector<LineVertex> lines;
			LineVertex line;
			line.Pos = glm::vec3();
			lines.push_back(line);
			line.Pos = glm::vec3(200.f, 0.f, 0.f);
			lines.push_back(line);
			auto& lineMeshComponent = m_Registry.emplace<LineMeshComponent>(entity, lines);
			lineMeshComponent.Color = glm::vec4(1.f, 0.f, 0.f, 1.f);
		}
		{
			const auto entity = m_Registry.create();
			m_Registry.emplace<TransformComponent>(entity);
			std::vector<LineVertex> lines;
			LineVertex line;
			line.Pos = glm::vec3();
			lines.push_back(line);
			line.Pos = glm::vec3(0.f, 0.f, 200.f);
			lines.push_back(line);
			auto& lineMeshComponent = m_Registry.emplace<LineMeshComponent>(entity, lines);
			lineMeshComponent.Color = glm::vec4(0.f, 0.f, 1.f, 1.f);
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
        //ball
        {
            const auto ballEntity = m_Registry.create();
            auto & transform = m_Registry.emplace<TransformComponent>(ballEntity);
            auto & ball  = m_Registry.emplace<BallComponent>(ballEntity);
            ball.Radius = 1.f;
            ball.Mass = 10.f;
            auto & velocity = m_Registry.emplace<VelocityComponent>(ballEntity);
            m_Registry.emplace<MeshComponent>(ballEntity,Utils::MakeBall(2.f,ball.Radius));
            m_Registry.emplace<TextureComponent>(ballEntity,"Assets/HappyTree.png");

            transform.Position.y += 30;
            transform.Position.x +=5;
            transform.Position.z -=5;
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
				static constexpr float speed = 20.f;
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
				if (Input::MouseButton(GLFW_MOUSE_BUTTON_2) == GLFW_PRESS
					|| Input::Key(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
					camera.Yaw(mouseDelta.x * mouseSpeed);
					camera.Pitch(mouseDelta.y * mouseSpeed);
				}
			}
		}
	}
	void Application::Simulate(double deltaTime) {
      {
                //set tranformation
                auto view = m_Registry.view<TransformComponent,VelocityComponent>();
                for(auto [entity,transform,velocity] : view.each()){
                    transform.Position += velocity.Velocity;

                    //teleport to top if falls under -20
                    if(transform.Position.y < -20.f){
                        transform.Position.y = 100.f;
                    }

                }
            }

        //loop trough ball and set velocity
        {
            Triangle triangle;
            //calculate ball velocity
            auto view = m_Registry.view<TransformComponent, BallComponent,VelocityComponent>();
            for (auto [entiry, transform, ball,velocity] : view.each()) {

                auto &terrain =  m_Registry.get<TerrainComponent>(m_TerrainEntity);
                for(auto & tri : terrain.Triangles){
                    if(Utils::isInside(transform.Position,tri)){
                        triangle = tri;
                        break;
                    }
                }
                velocity.Velocity = Physics::GetVelocityBall(triangle,transform.Position,ball,velocity.Velocity,deltaTime);

            }

        }
    //move stuff with velocity


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
				vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(RenderPipelineKeys::Basic), VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(MeshPushConstants), &constants);
				texture.Bind(commandBuffer);
				mesh.Draw(commandBuffer);
			}
		}

		{ // Line drawing
			m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Line);
			auto view = m_Registry.view<TransformComponent, LineMeshComponent>();
			for (auto [entity, transform, lineMesh] : view.each()) {
				LinePushConstants constants;
				constants.MVP = vp * transform.GetTransform();
				constants.Color = lineMesh.Color;
				vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(RenderPipelineKeys::Line), VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(LinePushConstants), &constants);
				lineMesh.Draw(commandBuffer);
			}
		}

		m_Renderer->EndRecording();
		m_Renderer->SubmitAndPresent();
	}
}
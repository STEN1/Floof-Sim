#include "Application.h"
#include "LoggerMacros.h"
#include "Timer.h"
#include "Components.h"
#include "Input.h"
#include "Utils.h"
#include "Physics.h"
#include <string>

namespace FLOOF {
	Application::Application() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(1600, 900, "Vulkan window", nullptr, nullptr);
		m_Renderer = new VulkanRenderer(m_Window);
		Input::Init(m_Window);
		Input::RegisterKeyPressCallback(GLFW_KEY_N, std::bind(&Application::DebugToggle, this));
        Input::RegisterKeyPressCallback(GLFW_KEY_R, std::bind(&Application::ResetBall, this));
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
		DebugInit();
		{
			m_TerrainEntity = m_Registry.create();
			auto& terrainTransform = m_Registry.emplace<TransformComponent>(m_TerrainEntity);
			auto vertexData = Utils::GetVisimVertexData("Assets/SimTerrain.visim");
			auto& terrain = m_Registry.emplace<TerrainComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<MeshComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<TextureComponent>(m_TerrainEntity, "Assets/HappyTree.png");
			terrain.PrintTriangleData();
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
        
        {	// Ball
            const auto ballEntity = m_Registry.create();
            auto & transform = m_Registry.emplace<TransformComponent>(ballEntity);
            auto & ball  = m_Registry.emplace<BallComponent>(ballEntity);
            ball.Radius = 0.01f;
            ball.Mass = 10.f;
            auto & velocity = m_Registry.emplace<VelocityComponent>(ballEntity);
            m_Registry.emplace<MeshComponent>(ballEntity,Utils::MakeBall(2.f,ball.Radius));
            m_Registry.emplace<TextureComponent>(ballEntity,"Assets/HappyTree.png");

            transform.Position.y += 0.125f;
            transform.Position.x +=0.f;
            transform.Position.z -=0.f;
        }

		{
			m_CameraEntity = m_Registry.create();
			glm::vec3 cameraPos(0.f, 0.1f, -0.4f);
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
			if (m_DebugDraw) {
				DebugClearLineBuffer();
			}
            if(deltaTime > 0.01f) deltaTime = 0.01f;
			Update(deltaTime);
			Simulate(deltaTime);
			if (m_DebugDraw) {
				DebugUpdateLineBuffer();
			}
			Draw();
		}

		m_Renderer->FinishAllFrames();
		m_Registry.clear();
		return 0;
	}
	void Application::Update(double deltaTime) {
		if (m_DebugDraw) {
			// World axis
			DebugDrawLine(glm::vec3(0.f), glm::vec3(100.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
			DebugDrawLine(glm::vec3(0.f), glm::vec3(0.f, 100.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
			DebugDrawLine(glm::vec3(0.f), glm::vec3(0.f, 0.f, 100.f), glm::vec3(0.f, 0.f, 1.f));

			// Terrain triangles
			TerrainComponent& triangleSurface = m_Registry.get<TerrainComponent>(m_TerrainEntity);
			glm::vec3 surfaceTriangleColor{ 1.f, 0.f, 1.f };
			for (auto& triangle : triangleSurface.Triangles) {
				DebugDrawTriangle(triangle, surfaceTriangleColor);
			}
		}

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
				static constexpr float speed = 2.f;
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
        //TODO make sure slowmotion is not active on delivery
        deltaTime *=.3f;

        //---- Set  ball Transform with velocity ----
        {
            auto view = m_Registry.view<TransformComponent,VelocityComponent>();
            for(auto [entity,transform,velocity] : view.each()){
                transform.Position += velocity.Velocity;
            }
        }

        // ---- Calculate ball velocity ----
        {
            int triangleIndex{-1};
            static int oldIndex{-1};
            bool bInside{false};
            auto view = m_Registry.view<TransformComponent, BallComponent,VelocityComponent>();
            for (auto [entiry, transform, ball,velocity] : view.each()) {
                auto &terrain =  m_Registry.get<TerrainComponent>(m_TerrainEntity);
                //find triangle under ball
                for(int i{0}; i < terrain.Triangles.size(); i++){
                    if(Utils::isInside(transform.Position,terrain.Triangles[i])){
                        triangleIndex = i;
                        bInside=true;
                        DebugDrawTriangle(terrain.Triangles[i],glm::vec3(0.f,255.f,0.f));
                        break;
                    }
                }
                if(!bInside)
                    triangleIndex=-1;

                //reflect velocity when hit corner
                if(oldIndex != triangleIndex && oldIndex != -1 && triangleIndex != -1){
                    glm::vec3 m = terrain.Triangles[oldIndex].N;
                    glm::vec3 n = terrain.Triangles[triangleIndex].N;
                    velocity.Velocity = Physics::GetReflectVelocity(velocity.Velocity, Physics::GetReflectionAngle(m,n));
                }
                oldIndex = triangleIndex;
                glm::vec3 a(0.f,-Math::Gravity,0.f);

                if(triangleIndex >= 0 && triangleIndex < terrain.Triangles.size()){
                    Triangle &triangle = terrain.Triangles[triangleIndex];
                    if(Physics::TriangleBallIntersect(triangle,transform.Position,ball.Radius)) {
                        a = Physics::GetAccelerationVector(triangle);
                        //move ball on top of triangle;
                        auto dist = (glm::dot(transform.Position-triangle.A,triangle.N));
                        transform.Position += glm::normalize(triangle.N)*(-dist+ball.Radius);
                    }
                }
                    velocity.Velocity += a*static_cast<float>(deltaTime)*static_cast<float>(deltaTime);
                    DebugDrawLine(transform.Position,transform.Position+velocity.Velocity*100.f,glm::vec3(0.f,0.f,255.f));
                    DebugDrawLine(transform.Position,transform.Position+a*0.01f,glm::vec3(255.f,0.f,0.f));
            }
        }
    }


	void Application::Draw() {
		auto commandBuffer = m_Renderer->StartRecording();

		// Camera setup
		auto extent = m_Renderer->GetExtent();
		CameraComponent& camera = m_Registry.get<CameraComponent>(m_CameraEntity);
		glm::mat4 vp = camera.GetVP(glm::radians(70.f), extent.width / (float)extent.height, 0.1f, 1000.f);

		{	// Geometry pass
			m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Basic);
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

		if (m_DebugDraw) { // Draw debug lines
			m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Line);
			auto view = m_Registry.view<LineMeshComponent>();
			for (auto [entity, lineMesh] : view.each()) {
				LinePushConstants constants;
				constants.MVP = vp;
				vkCmdPushConstants(commandBuffer, m_Renderer->GetPipelineLayout(RenderPipelineKeys::Line), VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(LinePushConstants), &constants);
				lineMesh.Draw(commandBuffer);
			}
		}

		m_Renderer->EndRecording();
		m_Renderer->SubmitAndPresent();
	}

	void Application::DebugInit() {
		// Size is max size of cmdBuffer updates
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdUpdateBuffer.html
		uint32_t size = 65536 / sizeof(LineVertex);
		m_DebugLineBuffer.resize(size);
		memset(m_DebugLineBuffer.data(), 0, m_DebugLineBuffer.size() * sizeof(LineVertex));
		m_DebugLineEntity = m_Registry.create();
		// Make the line mesh buffer as large as max update size by initializing with a buffer of that size.
		LineMeshComponent& lineMesh = m_Registry.emplace<LineMeshComponent>(m_DebugLineEntity, m_DebugLineBuffer);
	}

	void Application::DebugClearLineBuffer() {
		m_DebugLineBuffer.clear();
	}

	void Application::DebugUpdateLineBuffer() {
		auto commandBuffer = m_Renderer->AllocateBeginOneTimeCommandBuffer();
		auto& lineMesh = m_Registry.get<LineMeshComponent>(m_DebugLineEntity);
		lineMesh.UpdateBuffer(commandBuffer, m_DebugLineBuffer);
		m_Renderer->EndSubmitFreeCommandBuffer(commandBuffer);
	}

	void Application::DebugToggle()	{
		m_DebugDraw = !m_DebugDraw;
	}

	void Application::DebugDrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3 color) {
		LineVertex v;
		v.Color = color;
		v.Pos = start;
		m_DebugLineBuffer.push_back(v);
		v.Pos = end;
		m_DebugLineBuffer.push_back(v);
	}

	void Application::DebugDrawTriangle(const Triangle& triangle, const glm::vec3& color) {
		DebugDrawLine(triangle.A, triangle.B, color);
		DebugDrawLine(triangle.B, triangle.C, color);
		DebugDrawLine(triangle.C, triangle.A, color);
		glm::vec3 midPoint = (triangle.A + triangle.B + triangle.C) / 3.f;
		DebugDrawLine(midPoint, midPoint + (triangle.N * 0.02f), color);
	}

    void Application::ResetBall() {
        auto view = m_Registry.view<TransformComponent, BallComponent,VelocityComponent>();
        for(auto [entity, transform,ball, velocity]: view.each()){
            transform.Position = glm::vec3(0.f,0.125f,0.f);
           // transform.Position = glm::vec3(0.31f,0.225f,-0.2f);
            velocity.Velocity = glm::vec3(0.f);
        }

    }
}
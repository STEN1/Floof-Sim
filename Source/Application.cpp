#include "Application.h"
#include "LoggerMacros.h"
#include "Timer.h"
#include "Components.h"
#include "Input.h"
#include "Utils.h"
#include "Physics.h"
#include <string>
#include "stb_image.h"
#include "imgui_impl_glfw.h"

namespace FLOOF {
	Application::Application() {
		// Init glfw and create window
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(1600, 900, "Floof    FPS: 0.0", nullptr, nullptr);

		IMGUI_CHECKVERSION();
		m_ImguiContext = ImGui::CreateContext();
		ImGui::SetCurrentContext(m_ImguiContext);
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

		ImGui::StyleColorsDark();

		// Init Renderer and Imgui
		ImGui_ImplGlfw_InitForVulkan(m_Window, true);
		m_Renderer = new VulkanRenderer(m_Window);
		auto ImguiInitInfo = m_Renderer->GetImguiInitInfo();
		auto ImguiRenderPass = m_Renderer->GetImguiRenderPass();
		ImGui_ImplVulkan_Init(&ImguiInitInfo, ImguiRenderPass);
		auto commandBuffer = m_Renderer->AllocateBeginOneTimeCommandBuffer();
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		m_Renderer->EndSubmitFreeCommandBuffer(commandBuffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		// Upload icons for windows and taskbar
		GLFWimage images[3]{};
		int channels{};
		images[0].pixels = stbi_load("Assets/Icon16x16.png", &images[0].width, &images[0].height, &channels, 4);
		ASSERT(channels == 4);
		images[1].pixels = stbi_load("Assets/Icon32x32.png", &images[1].width, &images[1].height, &channels, 4);
		ASSERT(channels == 4);
		images[2].pixels = stbi_load("Assets/Icon48x48.png", &images[2].width, &images[2].height, &channels, 4);
		ASSERT(channels == 4);
		glfwSetWindowIcon(m_Window, 3, images);
		for (uint32_t i = 0; i < 3; i++) {
			stbi_image_free(images[i].pixels);
		}

		// Register key callbacks
		Input::Init(m_Window);
		Input::RegisterKeyPressCallback(GLFW_KEY_N, std::bind(&Application::DebugToggle, this));
		Input::RegisterKeyPressCallback(GLFW_KEY_R, std::bind(&Application::ResetBall, this));
		Input::RegisterKeyPressCallback(GLFW_KEY_F, std::bind(&Application::SpawnBall, this));
		Input::RegisterKeyPressCallback(GLFW_KEY_M, std::bind(&Application::DebugToggleDrawNormals, this));
		
		// Init Logger. Writes to specified log file.
		Utils::Logger::s_Logger = new Utils::Logger("Floof.log");
	}

	Application::~Application() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext(m_ImguiContext);

		MeshComponent::ClearMeshDataCache();
		TextureComponent::ClearTextureDataCache();

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
			auto& terrain = m_Registry.emplace<TerrainComponent>(m_TerrainEntity, Utils::GetVisimTriangles("Assets/SimTerrain.visim"));
			m_Registry.emplace<MeshComponent>(m_TerrainEntity, vertexData);
			m_Registry.emplace<TextureComponent>(m_TerrainEntity, "Assets/HappyTree.png");
			terrain.PrintTriangleData();
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
			auto& transform = m_Registry.emplace<TransformComponent>(ballEntity);
			auto& ball = m_Registry.emplace<BallComponent>(ballEntity);
			ball.Radius = 0.01f;
			ball.Mass = 2.05f;
			auto& velocity = m_Registry.emplace<VelocityComponent>(ballEntity);
			m_Registry.emplace<MeshComponent>(ballEntity, "Assets/Ball.obj");
			m_Registry.emplace<TextureComponent>(ballEntity, "Assets/BallTexture.png");

			transform.Position.y += 0.125f;
			transform.Position.x += 0.f;
			transform.Position.z -= 0.f;
			transform.Scale = glm::vec3(ball.Radius);
		}

		{
			const auto treeEntity = m_Registry.create();
			auto& transform = m_Registry.emplace<TransformComponent>(treeEntity);
			m_Registry.emplace<MeshComponent>(treeEntity, "Assets/HappyTree.obj");
			m_Registry.emplace<TextureComponent>(treeEntity, "Assets/HappyTree.png");
			transform.Position.x += 4.f;
		}

		{
			m_CameraEntity = m_Registry.create();
			glm::vec3 cameraPos(0.3f, 0.2f, 0.3f);
			auto& camera = m_Registry.emplace<CameraComponent>(m_CameraEntity, cameraPos);
			camera.Pitch(0.5f);
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
				std::string title = "Floof    FPS: " + std::to_string(fps);
				glfwSetWindowTitle(m_Window, title.c_str());
				titleBarUpdateTimer = 0.f;
				frameCounter = 0.f;
			}

			if (m_DebugDraw) {
				DebugClearLineBuffer();
			}

			if (deltaTime > 0.01f) {
				deltaTime = 0.01f;
			}

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

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

		{	// Rotate ball mesh.
			auto view = m_Registry.view<TransformComponent, MeshComponent, BallComponent>();
			for (auto [entity, transform, mesh, ball] : view.each()) {
				transform.Rotation.y += deltaTime;
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

		{	// Calculate ball velocity
			bool bInside{ false };

			auto view = m_Registry.view<TransformComponent, BallComponent, VelocityComponent>();
			for (auto [entity, transform, ball, velocity] : view.each()) {
				// Find triangle under ball
				auto& terrain = m_Registry.get<TerrainComponent>(m_TerrainEntity);
				for (int i{ 0 }; i < terrain.Triangles.size(); i++) {
					if (Utils::IsPointInsideTriangle(transform.Position, terrain.Triangles[i])) {
						if (Physics::PlaneBallIntersect(terrain.Triangles[i], transform.Position, ball.Radius)) {
							ball.TriangleIndex = i;
						}
						bInside = true;
						DebugDrawTriangle(terrain.Triangles[i], glm::vec3(0.f, 255.f, 0.f));
						break;
					}
				}

				if (!bInside) {
                    ball.TriangleIndex = -1;
				}

				// Reflect velocity when triangle index changes
				if (ball.LastTriangleIndex != ball.TriangleIndex && ball.LastTriangleIndex != -1 && ball.TriangleIndex != -1) {
					glm::vec3 m = terrain.Triangles[ball.LastTriangleIndex].N;
					glm::vec3 n = terrain.Triangles[ball.TriangleIndex].N;
					velocity.Velocity = Physics::GetReflectVelocity(velocity.Velocity, Physics::GetReflectionAngle(m, n));

					if (ball.TriangleIndex == 1 && ball.LastTriangleIndex == 0) {
						auto timeused = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_Ballspawntime).count();
						float seconds = timeused / 1000.f;
						std::string msg = "Time first triangle : ";
						msg += std::to_string(seconds);
						msg += " s";
						LOG_INFO(msg.c_str());
					}
				}
				ball.LastTriangleIndex = ball.TriangleIndex;
				glm::vec3 a(0.f, -Math::Gravity, 0.f);
				glm::vec3 af(a);

				if (ball.TriangleIndex >= 0 && ball.TriangleIndex < terrain.Triangles.size()) {
					Triangle& triangle = terrain.Triangles[ball.TriangleIndex];

					if (Physics::PlaneBallIntersect(triangle, transform.Position, ball.Radius)) {
						// Move ball on top of triangle;
						a = Physics::GetAccelerationVector(triangle);
						auto dist = (glm::dot(transform.Position - triangle.A, triangle.N));
						transform.Position += glm::normalize(triangle.N) * (-dist + ball.Radius);

						// Add friction
						if (glm::length(velocity.Velocity) > 0.f) {
							const float frictionConstant = triangle.FrictionConstant;
							auto frictionvec = -glm::normalize(velocity.Velocity) * (frictionConstant * ball.Mass);
							af = a + frictionvec;
							DebugDrawLine(transform.Position, transform.Position + frictionvec, glm::vec3(0.f, 125.f, 125.f));
						}
					}
				}


                //https://en.wikipedia.org/wiki/Verlet_integration
                //transform
                transform.Position = transform.Position + (velocity.Velocity*static_cast<float>(deltaTime)) +(af*(static_cast<float>(deltaTime)*static_cast<float>(deltaTime)*0.5f));
                //set velocity
                velocity.Velocity = velocity.Velocity +(af*static_cast<float>(deltaTime)*0.5f);
				//velocity.Velocity += af;
				DebugDrawLine(transform.Position, transform.Position + velocity.Velocity, glm::vec3(0.f, 0.f, 255.f));
				DebugDrawLine(transform.Position, transform.Position + a, glm::vec3(255.f, 0.f, 0.f));
				DebugDrawLine(transform.Position, transform.Position + af, glm::vec3(125.f, 125.f, 0.f));
			}
		}
	}


	void Application::Draw() {
		auto commandBuffer = m_Renderer->StartRecording();

		// Camera setup
		auto extent = m_Renderer->GetExtent();
		CameraComponent& camera = m_Registry.get<CameraComponent>(m_CameraEntity);
		glm::mat4 vp = camera.GetVP(glm::radians(70.f), extent.width / (float)extent.height, 0.1f, 1000.f);

		if (m_DrawNormals == false) {	// Geometry pass
			auto pipelineLayout = m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Basic);
			auto view = m_Registry.view<TransformComponent, MeshComponent, TextureComponent>();
			for (auto [entity, transform, mesh, texture] : view.each()) {
				MeshPushConstants constants;
				constants.MVP = vp * transform.GetTransform();
				constants.InvModelMat = glm::inverse(transform.GetTransform());
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(MeshPushConstants), &constants);

				texture.Bind(commandBuffer);

				mesh.Draw(commandBuffer);
			}
		} else if (m_DrawNormals == true) {
			auto pipelineLayout = m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Normal);
			auto view = m_Registry.view<TransformComponent, MeshComponent, TextureComponent>();
			for (auto [entity, transform, mesh, texture] : view.each()) {
				MeshPushConstants constants;
				constants.MVP = vp * transform.GetTransform();
				constants.InvModelMat = glm::inverse(transform.GetTransform());
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(MeshPushConstants), &constants);

				texture.Bind(commandBuffer);

				mesh.Draw(commandBuffer);
			}
		}

		if (m_DebugDraw) { // Draw debug lines
			auto pipelineLayout = m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Line);
			auto view = m_Registry.view<LineMeshComponent>();
			for (auto [entity, lineMesh] : view.each()) {
				LinePushConstants constants;
				constants.MVP = vp;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(LinePushConstants), &constants);

				lineMesh.Draw(commandBuffer);
			}
		}
		{
			if (m_ShowImguiDemo)
				ImGui::ShowDemoWindow(&m_ShowImguiDemo);

			ImGui::Render();
			ImDrawData* drawData = ImGui::GetDrawData();
			ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
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

	void Application::DebugToggle() {
		m_DebugDraw = !m_DebugDraw;
	}

	void Application::DebugToggleDrawNormals() {
		m_DrawNormals = !m_DrawNormals;
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
		m_Ballspawntime = std::chrono::high_resolution_clock::now();
		auto view = m_Registry.view<TransformComponent, BallComponent, VelocityComponent>();
		for (auto [entity, transform, ball, velocity] : view.each()) {
			transform.Position = glm::vec3(0.f, 0.125f, 0.f);
			//newpos = glm::vec3(0.f, 0.125f, 0.f);
			velocity.Velocity = glm::vec3(0.f);
            ball.LastTriangleIndex = -1;
            ball.TriangleIndex = -1;
		}

	}

	void Application::SpawnBall() {
		const auto ballEntity = m_Registry.create();
		auto& transform = m_Registry.emplace<TransformComponent>(ballEntity);
		auto& ball = m_Registry.emplace<BallComponent>(ballEntity);
		ball.Radius = 0.01f;
		ball.Mass = 2.05f;
		auto& velocity = m_Registry.emplace<VelocityComponent>(ballEntity);
		m_Registry.emplace<MeshComponent>(ballEntity, "Assets/Ball.obj");
		m_Registry.emplace<TextureComponent>(ballEntity, "Assets/BallTexture.png");

		auto& camera = m_Registry.get<CameraComponent>(m_CameraEntity);
		transform.Position = camera.Position;
		transform.Scale = glm::vec3(ball.Radius);
	}
}

//test
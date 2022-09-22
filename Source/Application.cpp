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
#include "LasLoader.h"
#include "Octree.h"

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
		Input::RegisterKeyPressCallback(GLFW_KEY_R, std::bind(&Application::ResetBall, this));
		//Input::RegisterKeyPressCallback(GLFW_KEY_F, std::bind(&Application::SpawnBall, this));
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

            const auto PointCloudEntity = m_Registry.create();
            m_Registry.emplace<PointCloudComponent>(PointCloudEntity, LasLoader("Assets/france.txt").GetVertexData());

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
            auto& time = m_Registry.emplace<TimeComponent>(ballEntity);
            time.CreationTime = Timer::GetTime();
            time.LastPoint=time.CreationTime;
			ball.Radius = 0.01f;
			ball.Mass = .5f;
            ball.CollisionSphere.radius = ball.Radius;
            ball.CollisionSphere.pos = transform.Position;
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

			DebugClearDebugData();

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
            // World axis
            if (m_BDebugLines[DebugLine::WorldAxis]) {
                DebugDrawLine(glm::vec3(0.f), glm::vec3(10.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
                DebugDrawLine(glm::vec3(0.f), glm::vec3(0.f, 10.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
                DebugDrawLine(glm::vec3(0.f), glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 0.f, 1.f));
            }

			// Terrain triangles
            if(m_BDebugLines[DebugLine::TerrainTriangle]) {
                TerrainComponent &triangleSurface = m_Registry.get<TerrainComponent>(m_TerrainEntity);
                glm::vec3 surfaceTriangleColor{1.f, 0.f, 1.f};
                for (auto &triangle: triangleSurface.Triangles) {
                    DebugDrawTriangle(triangle, surfaceTriangleColor);
                }
            }
			// Closest point on triangle to ball center
			if (m_BDebugLines[DebugLine::ClosestPointToBall]) {
				auto& terrain = m_Registry.get<TerrainComponent>(m_TerrainEntity);
				auto view = m_Registry.view<BallComponent>();
				static constexpr glm::vec3 pointColor = glm::vec3(1.f);
				for (auto [entity, ball] : view.each()) {
					for (auto& triangle : terrain.Triangles) {
						glm::vec3 start = CollisionShape::ClosestPointToPointOnTriangle(ball.CollisionSphere.pos, triangle);
						glm::vec3 end = start + (triangle.N * 0.1f);
						DebugDrawLine(start, end, pointColor);
					}
				}
			}


            // Terrain triangles
            if (m_BDebugLines[DebugLine::TerrainTriangle]) {
                TerrainComponent &triangleSurface = m_Registry.get<TerrainComponent>(m_TerrainEntity);
                glm::vec3 surfaceTriangleColor{1.f, 0.f, 1.f};
                for (auto &triangle: triangleSurface.Triangles) {
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
		
		{	// UI
			if (m_ShowImguiDemo)
				ImGui::ShowDemoWindow(&m_ShowImguiDemo);

			ImGui::Begin("Utils");
			if (ImGui::Button("Spawn ball"))
				SpawnBall();
            if(ImGui::Button("Reset Ball"))
                ResetBall();
            if(ImGui::Button("DrawNormals")){
                DebugToggleDrawNormals();
            }
            ImGui::SliderFloat("Deltatime Modifer",&m_DeltaTimeModifier, 0.f, 1.f);
			ImGui::End();


            ImGui::Begin("Toggle DebugLines");
            if(ImGui::Button("All Debug Lines"))
                DebugToggleAllLines();
                if(ImGui::Button("Velocity Vector"))
                m_BDebugLines[DebugLine::Velocity]  = !m_BDebugLines[DebugLine::Velocity];
            if(ImGui::Button("Force Vector"))
                m_BDebugLines[DebugLine::Force]  = !m_BDebugLines[DebugLine::Force];
            if(ImGui::Button("Friction Vector"))
                m_BDebugLines[DebugLine::Friction]  = !m_BDebugLines[DebugLine::Friction];
            if(ImGui::Button("Collision shapes"))
                m_BDebugLines[DebugLine::CollisionShape]  = !m_BDebugLines[DebugLine::CollisionShape];
            if(ImGui::Button("Terrain Triangles"))
                m_BDebugLines[DebugLine::TerrainTriangle]  = !m_BDebugLines[DebugLine::TerrainTriangle];
            if(ImGui::Button("World Axis"))
                m_BDebugLines[DebugLine::WorldAxis]  = !m_BDebugLines[DebugLine::WorldAxis];
			if (ImGui::Button("Closest triangle point"))
				m_BDebugLines[DebugLine::ClosestPointToBall] = !m_BDebugLines[DebugLine::ClosestPointToBall];
            if(ImGui::Button("Gravitational Pull"))
                m_BDebugLines[DebugLine::GravitationalPull]  = !m_BDebugLines[DebugLine::GravitationalPull];
            if(ImGui::Button("Path Trace"))
                m_BDebugLines[DebugLine::Path]  = !m_BDebugLines[DebugLine::Path];
            if(ImGui::Button("Point Cloud")){
                HidePointCloud();
            }
            //ImGui::Checkbox(("World Axis"), &m_BDebugLines["WorldAxis"]);
            ImGui::End();

            ImGui::Begin("Oppgaver");
            static int raincount = 10;
            ImGui::SliderInt("Rain Ball Count", &raincount, 0, 100);
            if(ImGui::Button("Spawn Rain"))
                SpawnRain(raincount);
            ImGui::End();
		}
	}
	void Application::Simulate(double deltaTime) {

        deltaTime *= m_DeltaTimeModifier;

		AABB worldExtents{};
		worldExtents.extent = glm::vec3(1.f);
		worldExtents.pos = glm::vec3(0.f);
		Octree octree(worldExtents);

		{
			auto view = m_Registry.view<TransformComponent, VelocityComponent, BallComponent>();
			for (auto [entity, transform, velocity, ball] : view.each()) {
				octree.Insert(Octree::CollisionObject(&ball.CollisionSphere, transform, velocity, ball));
			}

			std::vector<Octree*> leafNodes;
			octree.GetActiveLeafNodes(leafNodes);

			for (auto& node : leafNodes) {
				auto aabb = node->GetAABB();
				DebugDrawAABB(aabb.pos, aabb.extent);
			}
		}

		{	// Calculate ball velocity

            glm::vec3 acc(0.f, -Math::Gravity, 0.f);
            glm::vec3 fri(0.f);

			std::vector<std::pair<Octree::CollisionObject, Octree::CollisionObject>> collisionPairs;
			octree.GetCollisionPairs(collisionPairs);

			for (auto& [obj1, obj2] : collisionPairs) {
				auto& collidingTransform1 = obj1.Transform;
				auto& collidingVelocity1 = obj1.Velocity;
				auto& collidingBall1 = obj1.Ball;

				auto& collidingTransform2 = obj2.Transform;
				auto& collidingVelocity2 = obj2.Velocity;
				auto& collidingBall2 = obj2.Ball;

				//calculate velocity when collision with another ball
                glm::vec3 contactNormal{Math::GetSafeNormal()};
                if(glm::length(collidingTransform1.Position-collidingTransform2.Position) != 0 )
                    contactNormal = glm::normalize(collidingTransform2.Position - collidingTransform1.Position);

				auto combinedMass = collidingBall2.Mass + collidingBall1.Mass;
				auto elasticity = collidingBall2.Elasticity * collidingBall1.Elasticity;
				auto relVelocity = collidingVelocity2.Velocity - collidingVelocity1.Velocity;

				float moveangle = glm::dot(relVelocity, contactNormal);
				float j = -(1.f + elasticity) * moveangle / (1.f / combinedMass);
				if (moveangle >= 0.f) { // moves opposite dirrections;
					j = 0.f;
				}
				const glm::vec3 vecImpulse = j * contactNormal;
				collidingVelocity2.Velocity += vecImpulse / combinedMass;

                //move ball when overlapping
                float dist = glm::length(collidingTransform1.Position-collidingTransform2.Position);
                if(dist < (collidingBall1.Radius+collidingBall2.Radius)) {
                    collidingTransform2.Position += contactNormal * ((collidingBall1.Radius + collidingBall2.Radius) - dist);
                }
			}

			auto view = m_Registry.view<TransformComponent, BallComponent, VelocityComponent, TimeComponent>();
			auto& terrain = m_Registry.get<TerrainComponent>(m_TerrainEntity);
			for (auto [entity, transform, ball, velocity,time] : view.each()) {

                velocity.Force = Math::GravitationalPull * ball.Mass;

                bool foundCollision = false;
				for (int i{ 0 }; i < terrain.Triangles.size(); i++) {

                    //------ new collision ------
                    if(ball.CollisionSphere.Intersect(&terrain.Triangles[i])){
                        ball.TriangleIndex = i;

                        //first hit on ground
                        if(ball.Path.empty()){
                            time.LastPoint = Timer::GetTime();
                            ball.Path.emplace_back(transform.Position);
                        }


                        //draw debug triangle
                        if(m_BDebugLines[DebugLine::TerrainTriangle])
                            DebugDrawTriangle(terrain.Triangles[i], glm::vec3(0.f, 255.f, 0.f));

                        Triangle& triangle = terrain.Triangles[ball.TriangleIndex];

                        glm::vec3 norm = glm::normalize(CollisionShape::ClosestPointToPointOnTriangle(transform.Position, triangle)-transform.Position);

                        float moveangle = glm::dot(velocity.Velocity,norm);
                        float j = -(1.f+ball.Elasticity) * moveangle / (1.f/ball.Mass);
                        const glm::vec3 vecImpulse = j*norm;
                        velocity.Velocity += vecImpulse / ball.Mass;

                        // Add friction
                       if (glm::length(velocity.Velocity) > 0.f) {
                           const float frictionConstant = triangle.FrictionConstant;
                           fri = -glm::normalize(velocity.Velocity) * (frictionConstant * ball.Mass);
                        }
                        ball.LastTriangleIndex = ball.TriangleIndex;
                        foundCollision = true;

                        auto dist = (glm::dot(transform.Position - triangle.A, triangle.N));
                        transform.Position += glm::normalize(triangle.N) * (-dist + ball.Radius);
                    }

                    if(!foundCollision)
                        ball.TriangleIndex = -1;
				}

                //https://en.wikipedia.org/wiki/Verlet_integration
                transform.Position += (velocity.Velocity*static_cast<float>(deltaTime)) +(((velocity.Force)+fri)*(static_cast<float>(deltaTime)*static_cast<float>(deltaTime)*0.5f));
                velocity.Velocity += (((velocity.Force/ball.Mass)+fri)*static_cast<float>(deltaTime)*0.5f);

                //set collision sphere location
                ball.CollisionSphere.pos = transform.Position;

                const float pointIntervall{0.1f};
                //save ball path
                if(Timer::GetTimeSince(time.LastPoint) >= pointIntervall && !ball.Path.empty()){
                    time.LastPoint = Timer::GetTime();
                    ball.Path.emplace_back(transform.Position);
                }

                if(m_BDebugLines[DebugLine::Path])
                    DebugDrawPath(ball.Path);
                //draw debug lines
                if(m_BDebugLines[DebugLine::Friction])
                    DebugDrawLine(transform.Position, transform.Position + fri, glm::vec3(0.f, 125.f, 125.f));
                if(m_BDebugLines[DebugLine::CollisionShape])
                    DebugDrawSphere(ball.CollisionSphere.pos,ball.CollisionSphere.radius);
                if(m_BDebugLines[DebugLine::Velocity])
				    DebugDrawLine(transform.Position, transform.Position + velocity.Velocity, glm::vec3(0.f, 0.f, 255.f));
				if(m_BDebugLines[DebugLine::Force])
                    DebugDrawLine(transform.Position, transform.Position + velocity.Force, glm::vec3(255.f, 0.f, 0.f));
                if(m_BDebugLines[DebugLine::GravitationalPull])
                    DebugDrawLine(transform.Position,transform.Position+Math::GravitationalPull, glm::vec3(255.f, 255.f, 255.f));



                //reset force
                velocity.Force = glm::vec3(0.f);

                //move ball when they fall and reset path
                if(transform.Position.y <= -0.5f){
                    const double minX{0};
                    const double maxX{0.6};
                    const double minZ{-0.4};
                    const double maxZ{0.0};
                    glm::vec3 loc(Math::RandDouble(minX,maxX),0.3f,Math::RandDouble(minZ,maxZ));
                    transform.Position = loc;
                    velocity.Velocity = glm::vec3(0.f);
                    ball.Path.clear();
                }

            }
		}
	}


	void Application::Draw() {
		auto commandBuffer = m_Renderer->StartRecording();

		// Camera setup
		auto extent = m_Renderer->GetExtent();
		CameraComponent& camera = m_Registry.get<CameraComponent>(m_CameraEntity);
		glm::mat4 vp = camera.GetVP(glm::radians(70.f), extent.width / (float)extent.height, 0.01f, 100.f);

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
		} else if (m_DrawNormals) { // Debug drawing of normals for geometry
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

        if(m_BShowPointcloud)
		{	// Draw point cloud
			auto pipelineLayout = m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Point);
			auto view = m_Registry.view<PointCloudComponent>();
			for (auto [entity, cloud] : view.each()) {
				ColorPushConstants constants;
				constants.MVP = vp;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(ColorPushConstants), &constants);
				cloud.Draw(commandBuffer);
			}
		}

		if (m_DebugDraw) { // Draw debug lines
			auto pipelineLayout = m_Renderer->BindGraphicsPipeline(commandBuffer, RenderPipelineKeys::Line);

			ColorPushConstants constants;
			constants.MVP = vp;
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(ColorPushConstants), &constants);

			auto& lineMesh = m_Registry.get<LineMeshComponent>(m_DebugLineEntity);
			lineMesh.Draw(commandBuffer);

			auto& sphereMesh = m_Registry.get<LineMeshComponent>(m_DebugSphereEntity);
			for (auto& transform : m_DebugSphereTransforms) {
				constants.MVP = vp * transform;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(ColorPushConstants), &constants);
				sphereMesh.Draw(commandBuffer);
			}

			auto& boxMesh = m_Registry.get<LineMeshComponent>(m_DebugAABBEntity);
			for (auto& transform : m_DebugAABBTransforms) {
				constants.MVP = vp * transform;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(ColorPushConstants), &constants);
				boxMesh.Draw(commandBuffer);
			}
		}

		{	// Draw ImGui
			ImGui::Render();
			ImDrawData* drawData = ImGui::GetDrawData();
			ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
		}

		m_Renderer->EndRecording();
		m_Renderer->SubmitAndPresent();
	}

	void Application::DebugInit() {
		m_DebugLineBuffer.resize(100000);
		m_DebugLineEntity = m_Registry.create();
		m_Registry.emplace<LineMeshComponent>(m_DebugLineEntity, m_DebugLineBuffer);
		m_Registry.emplace<DebugComponent>(m_DebugLineEntity);

		m_DebugSphereEntity = m_Registry.create();
		m_Registry.emplace<LineMeshComponent>(m_DebugSphereEntity, Utils::LineVertexDataFromObj("Assets/Ball.obj"));
		m_Registry.emplace<DebugComponent>(m_DebugSphereEntity);

		m_DebugAABBEntity = m_Registry.create();
		m_Registry.emplace<LineMeshComponent>(m_DebugAABBEntity, Utils::MakeBox(glm::vec3(1.f), glm::vec3(1.f, 0.f, 0.f)));
		m_Registry.emplace<DebugComponent>(m_DebugAABBEntity);

		m_BDebugLines[DebugLine::Velocity] = false;
		m_BDebugLines[DebugLine::Friction] = false;
		m_BDebugLines[DebugLine::Acceleration] = false;
		m_BDebugLines[DebugLine::CollisionShape] = false;
		m_BDebugLines[DebugLine::WorldAxis] = true;
		m_BDebugLines[DebugLine::TerrainTriangle] = false;
		m_BDebugLines[DebugLine::ClosestPointToBall] = false;
        m_BDebugLines[DebugLine::GravitationalPull] = false;
        m_BDebugLines[DebugLine::Path] = true;
	}

	void Application::DebugClearLineBuffer() {
		m_DebugLineBuffer.clear();
	}

	void Application::DebugClearSpherePositions() {
		m_DebugSphereTransforms.clear();
	}

	void Application::DebugClearAABBTransforms() {
		m_DebugAABBTransforms.clear();
	}

	void Application::DebugClearDebugData() {
		DebugClearLineBuffer();
		DebugClearSpherePositions();
		DebugClearAABBTransforms();
	}

	void Application::DebugUpdateLineBuffer() {
		auto commandBuffer = m_Renderer->AllocateBeginOneTimeCommandBuffer();
		auto& lineMesh = m_Registry.get<LineMeshComponent>(m_DebugLineEntity);
		lineMesh.UpdateBuffer(m_DebugLineBuffer);
		m_Renderer->EndSubmitFreeCommandBuffer(commandBuffer);
	}

	void Application::DebugToggleDrawNormals() {
		m_DrawNormals = !m_DrawNormals;
	}

	void Application::DebugDrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3 color) {
		ColorVertex v;
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

	void Application::DebugDrawSphere(const glm::vec3& pos, float radius) {
		glm::mat4 transform = glm::translate(pos);
		transform = glm::scale(transform, glm::vec3(radius));
		m_DebugSphereTransforms.push_back(transform);
	}

	void Application::DebugDrawAABB(const glm::vec3& pos, const glm::vec3& extents) {
		glm::mat4 transform(1.f);
		transform[3].x = pos.x;
		transform[3].y = pos.y;
		transform[3].z = pos.z;

		transform[0].x = extents.x;
		transform[1].y = extents.y;
		transform[2].z = extents.z;
		m_DebugAABBTransforms.push_back(transform);
	}

	void Application::ResetBall() {
		auto view = m_Registry.view<TransformComponent, BallComponent, VelocityComponent>();
		for (auto [entity, transform, ball, velocity] : view.each()) {
			transform.Position = glm::vec3(0.f, 0.125f, 0.f);
			//newpos = glm::vec3(0.f, 0.125f, 0.f);
			velocity.Velocity = glm::vec3(0.f);
            ball.CollisionSphere.radius = ball.Radius;
            ball.CollisionSphere.pos = transform.Position;
            ball.LastTriangleIndex = -1;
            ball.TriangleIndex = -1;
		}

	}

	void Application::SpawnBall() {
		const auto ballEntity = m_Registry.create();
		auto& transform = m_Registry.emplace<TransformComponent>(ballEntity);
		auto& ball = m_Registry.emplace<BallComponent>(ballEntity);
        auto& time = m_Registry.emplace<TimeComponent>(ballEntity);

        ball.Radius = 0.01f;
		ball.Mass = 0.50f;
        time.CreationTime = Timer::GetTime();
        time.LastPoint = time.CreationTime;

		auto& velocity = m_Registry.emplace<VelocityComponent>(ballEntity);
		m_Registry.emplace<MeshComponent>(ballEntity, "Assets/Ball.obj");
		m_Registry.emplace<TextureComponent>(ballEntity, "Assets/BallTexture.png");

		auto& camera = m_Registry.get<CameraComponent>(m_CameraEntity);
		transform.Position = camera.Position;
		transform.Scale = glm::vec3(ball.Radius);
        
        ball.CollisionSphere.radius = ball.Radius;
        ball.CollisionSphere.pos = transform.Position;
	}

    const void Application::SpawnRain(const int count) {

        const double minX{0};
        const double maxX{0.6};
        const double minZ{-0.4};
        const double maxZ{0.0};

        for(int i = 0; i < count; i++){
            float rad = Math::RandDouble(0.007f,0.01f);
            float mass = rad*10.f;
                glm::vec3 loc(Math::RandDouble(minX,maxX),0.3f,Math::RandDouble(minZ,maxZ));
                SpawnBall(loc, rad, mass);
        }
    }

    const void Application::SpawnBall(glm::vec3 location, const float radius, const float mass) {
        const auto ballEntity = m_Registry.create();
        auto& transform = m_Registry.emplace<TransformComponent>(ballEntity);
        auto& ball = m_Registry.emplace<BallComponent>(ballEntity);
        auto& time = m_Registry.emplace<TimeComponent>(ballEntity);

        ball.Radius = radius;
        ball.Mass = mass;
        time.CreationTime = Timer::GetTime();
        time.LastPoint=time.CreationTime;

        auto& velocity = m_Registry.emplace<VelocityComponent>(ballEntity);
        m_Registry.emplace<MeshComponent>(ballEntity, "Assets/Ball.obj");
        m_Registry.emplace<TextureComponent>(ballEntity, "Assets/BallTexture.png");

        transform.Position = location;
        transform.Scale = glm::vec3(ball.Radius);

        ball.CollisionSphere.radius = ball.Radius;
        ball.CollisionSphere.pos = transform.Position;

    }

    void Application::DebugToggleAllLines() {
        m_DebugDraw = ! m_DebugDraw;
    }

    void Application::DebugDrawPath(std::vector<glm::vec3> &path) {
        for(int i {1}; i < path.size(); i++){
            DebugDrawLine(path[i-1],path[i],glm::vec3(255.f,255.f,255.f));
        }

    }

    void Application::HidePointCloud() {
        m_BShowPointcloud = !m_BShowPointcloud;
    }
}

//test
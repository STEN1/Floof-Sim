#pragma once
#include "Math.h"

#include "VulkanRenderer.h"
#include "Floof.h"
#include "Physics.h"


namespace FLOOF {
	struct TransformComponent {
		inline static constexpr bool in_place_delete = true;

		TransformComponent* Parent = nullptr;

		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Rotation = glm::vec3(0.f);
		glm::vec3 Scale = glm::vec3(1.f);

		glm::mat4 GetLocalTransform() const {
			return glm::translate(Position)
				* glm::toMat4(glm::quat(Rotation))
				* glm::scale(Scale);
		}

		glm::mat4 GetTransform() const {
			TransformComponent* parent = Parent;

			glm::mat4 transform = GetLocalTransform();

			while (parent) {
				transform = parent->GetLocalTransform() * transform;
				parent = parent->Parent;
			}

			return transform;
		}
	};

	struct MeshComponent {
		MeshComponent(const std::string& objPath);
		MeshComponent(const std::vector<MeshVertex>& vertexData, const std::vector<uint32_t>& indexData);
		MeshComponent(const std::vector<MeshVertex>& vertexData);
		~MeshComponent();

		void Draw(VkCommandBuffer commandBuffer);

		struct MeshData {
			VulkanBuffer VertexBuffer{};
			VulkanBuffer IndexBuffer{};
			uint32_t VertexCount{};
			uint32_t IndexCount{};
		};

		MeshData Data{};

		static void ClearMeshDataCache();
	private:
		bool m_IsCachedMesh = false;
		inline static std::unordered_map<std::string, MeshData> s_MeshDataCache;
	};

	struct LineMeshComponent {
		LineMeshComponent(const std::vector<ColorVertex>& vertexData);
		~LineMeshComponent();

		void Draw(VkCommandBuffer commandBuffer);
		// Dont call this in a renderpass
		void UpdateBuffer(VkCommandBuffer commandBuffer, const std::vector<ColorVertex>& vertexData);
		VulkanBuffer VertexBuffer{};
		uint32_t VertexCount{};
		uint32_t MaxVertexCount{};
	};

	struct PointCloudComponent {
		PointCloudComponent(const std::vector<ColorVertex>& vertexData);
		~PointCloudComponent();

		VulkanBuffer VertexBuffer{};
		uint32_t VertexCount{};
	};

	struct TextureComponent {
		TextureComponent(const std::string& path);
		~TextureComponent();

		void Bind(VkCommandBuffer commandBuffer);

		struct TextureData {
			VulkanCombinedTextureSampler CombinedTextureSampler{};
			VkDescriptorSet DesctriptorSet{};
		};

		TextureData Data{};

		static void ClearTextureDataCache();
	private:
		inline static std::unordered_map<std::string, TextureData> s_TextureDataCache;
	};

	struct CameraComponent {
		CameraComponent(glm::vec3 position);
		glm::mat4 GetVP(float fov, float aspect, float near, float far);
		void MoveForward(float amount);
		void MoveRight(float amount);
		void Pitch(float amount);
		void Yaw(float amount);
		glm::vec3 Position;
		glm::vec3 Forward;
		glm::vec3 Up;
		glm::vec3 Right;
		float FOV = 1.f;
		float Near = 0.1f;
		float Far = 100.f;
		float Aspect = 16.f / 9.f;
	};

	struct TerrainComponent {
		TerrainComponent(std::vector<Triangle>&& vertexData);
		std::vector<Triangle> Triangles;
		void PrintTriangleData();
	};

    struct BallComponent {
        Sphere CollisionSphere;
        float Radius; // TODO dobbel lagring av radius !! fix
        float Mass;
        int LastTriangleIndex{-1};
        int TriangleIndex{-1};
    };
    struct VelocityComponent {
        glm::vec3 Velocity;

    };
	struct DebugComponent {};
}


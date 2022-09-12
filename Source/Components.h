#pragma once
#include "Math.h"

#include "VulkanRenderer.h"
#include "Floof.h"


namespace FLOOF {
	struct TransformComponent {
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Rotation = glm::vec3(0.f);
		glm::vec3 Scale = glm::vec3(1.f);

		glm::mat4 GetTransform() const {
			return glm::translate(Position)
				* glm::toMat4(glm::quat(Rotation))
				* glm::scale(Scale);
		}
	};

	struct MeshComponent {
		MeshComponent(const std::string& objPath);
		MeshComponent(const std::vector<MeshVertex>& vertexData, const std::vector<uint32_t>& indexData);
		MeshComponent(const std::vector<MeshVertex>& vertexData);
		~MeshComponent();

		void Draw(VkCommandBuffer commandBuffer);

		VulkanBuffer VertexBuffer{};
		VulkanBuffer IndexBuffer{};
		uint32_t VertexCount{};
		uint32_t IndexCount{};
	};

	struct LineMeshComponent {
		LineMeshComponent(const std::vector<LineVertex>& vertexData);
		~LineMeshComponent();

		void Draw(VkCommandBuffer commandBuffer);

		VulkanBuffer VertexBuffer{};
		uint32_t VertexCount{};
		glm::vec4 Color{};
	};

	struct TextureComponent {
		TextureComponent(const std::string& path);
		~TextureComponent();

		void Bind(VkCommandBuffer commandBuffer);

		VulkanCombinedTextureSampler CombinedTextureSampler{};
		VkDescriptorSet DesctriptorSet{};
	};

	struct CameraComponent {
		CameraComponent(glm::vec3 position);
		glm::mat4 GetVP(float fov, float aspect, float near, float far);
		void MoveForward(float amount);
		void MoveRight(float amount);
		void Pitch(float amount);
		void Yaw(float amount);
		glm::vec3 Forward;
		glm::vec3 Up;
		glm::vec3 Position;
	};

	struct TerrainComponent {
		TerrainComponent(const std::vector<MeshVertex>& vertexData);
		std::vector<Triangle> TerrainTriangles;
	};

    struct BallCompoonent {
        float Radius;
    };
    struct VelocityComponent {
        glm::vec3 Velocity;

    };
}


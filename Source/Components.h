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
		MeshComponent(const std::string& path);
		~MeshComponent();

		void Draw(VkCommandBuffer commandBuffer);

		VulkanBuffer VertexBuffer{};
		VulkanBuffer IndexBuffer{};
	};

	struct TextureComponent {
		TextureComponent(const std::string& path);
		~TextureComponent();
		void Bind(VkCommandBuffer commandBuffer);

		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkImage m_Image = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		VmaAllocationInfo m_AllocationInfo{};

		VkDescriptorSet DesctriptorSet{};
	};

	struct CameraComponent {};
}
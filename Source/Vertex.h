#pragma once

#include <vulkan/vulkan.h>
#include <array>

#include "Math.h"

namespace FLOOF {
	struct Vertex {
		glm::vec3 Pos;
		glm::vec3 Normal;
		glm::vec2 UV;

		bool operator == (const Vertex& other) const {
			return Pos == other.Pos && Normal == other.Normal && UV == other.UV;
		}

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, Pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, Normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, UV);

			return attributeDescriptions;
		}
	};
    struct Triangle {
        glm::vec3 A;
        glm::vec3 B;
        glm::vec3 C;
        glm::vec3 N;
    };
}
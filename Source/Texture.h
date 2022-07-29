#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace FLOOF {
	class Texture {
	public:
		Texture(const std::string& path);
		~Texture();
		void Bind(VkCommandBuffer commandBuffer);
	private:
		VkImageView m_ImageView{};
		VkImage m_Image{};
		VmaAllocation m_Allocation{};
		VmaAllocationInfo m_AllocationInfo{};

		VkSampler m_Sampler{};
	};
}
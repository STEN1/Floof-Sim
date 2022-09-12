#include "Components.h"

#include <cstring>
#include "stb_image.h"
#include "ObjLoader.h"

namespace FLOOF {
	TextureComponent::TextureComponent(const std::string& path) {
		auto renderer = VulkanRenderer::Get();
		// Load texture
		int xWidth, yHeight, channels;
		stbi_set_flip_vertically_on_load(true);
		auto* data = stbi_load(path.c_str(), &xWidth, &yHeight, &channels, 0);
		uint32_t size = xWidth * yHeight * channels;
		ASSERT(channels == 4);

		// staging buffer
		VkBufferCreateInfo stagingCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		stagingCreateInfo.size = size;
		stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
		stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		stagingBufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
			VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer stagingBuffer{};
		VmaAllocation stagingBufferAlloc{};
		VmaAllocationInfo stagingBufferAllocInfo{};
		vmaCreateBuffer(renderer->m_Allocator, &stagingCreateInfo, &stagingBufAllocCreateInfo, &stagingBuffer,
			&stagingBufferAlloc, &stagingBufferAllocInfo);

		ASSERT(stagingBufferAllocInfo.pMappedData != nullptr);
		memcpy(stagingBufferAllocInfo.pMappedData, data, size);
		stbi_image_free(data);

		// Image
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = xWidth;
		imageInfo.extent.height = yHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo imageAllocCreateInfo = {};
		imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		vmaCreateImage(renderer->m_Allocator, &imageInfo, &imageAllocCreateInfo, &CombinedTextureSampler.Image,
			&CombinedTextureSampler.Allocation, &CombinedTextureSampler.AllocationInfo);

		// copy image from staging buffer to image buffer(gpu only memory)
		renderer->CopyBufferToImage(stagingBuffer, CombinedTextureSampler.Image, xWidth, yHeight);

		// free staging buffer
		vmaDestroyBuffer(renderer->m_Allocator, stagingBuffer, stagingBufferAlloc);

		// create image view
		VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		textureImageViewInfo.image = CombinedTextureSampler.Image;
		textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		textureImageViewInfo.subresourceRange.baseMipLevel = 0;
		textureImageViewInfo.subresourceRange.levelCount = 1;
		textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
		textureImageViewInfo.subresourceRange.layerCount = 1;
		vkCreateImageView(renderer->m_LogicalDevice, &textureImageViewInfo, nullptr, &CombinedTextureSampler.ImageView);

		// sampler
		VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.f;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = FLT_MAX;
		vkCreateSampler(renderer->m_LogicalDevice, &samplerInfo, nullptr, &CombinedTextureSampler.Sampler);

		// Get descriptor set and point it to data.
		DesctriptorSet = renderer->AllocateTextureDescriptorSet();

		VkDescriptorImageInfo descriptorImageInfo{};
		descriptorImageInfo.sampler = CombinedTextureSampler.Sampler;
		descriptorImageInfo.imageView = CombinedTextureSampler.ImageView;
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = DesctriptorSet;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &descriptorImageInfo;

		vkUpdateDescriptorSets(renderer->m_LogicalDevice, 1, &writeDescriptorSet, 0, nullptr);
	}
	TextureComponent::~TextureComponent() {
		auto renderer = VulkanRenderer::Get();

		renderer->FreeTextureDescriptorSet(DesctriptorSet);
		vkDestroyImageView(renderer->m_LogicalDevice, CombinedTextureSampler.ImageView, nullptr);
		vmaDestroyImage(renderer->m_Allocator, CombinedTextureSampler.Image, CombinedTextureSampler.Allocation);
		vkDestroySampler(renderer->m_LogicalDevice, CombinedTextureSampler.Sampler, nullptr);
	}
	void TextureComponent::Bind(VkCommandBuffer commandBuffer) {
		auto renderer = VulkanRenderer::Get();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->GetPipelineLayout(),
			0, 1, &DesctriptorSet, 0, 0);
	}
	MeshComponent::MeshComponent(const std::string& path) {
		auto* renderer = VulkanRenderer::Get();

		auto [vertexData, indexData] = ObjLoader(path).GetIndexedData();
		VertexBuffer = renderer->CreateVertexBuffer(vertexData);
		IndexBuffer = renderer->CreateIndexBuffer(indexData);
		VertexCount = vertexData.size();
		IndexCount = indexData.size();

	}
	MeshComponent::MeshComponent(const std::vector<MeshVertex>& vertexData, const std::vector<uint32_t>& indexData)	{
		auto* renderer = VulkanRenderer::Get();

		VertexBuffer = renderer->CreateVertexBuffer(vertexData);
		IndexBuffer = renderer->CreateIndexBuffer(indexData);
		VertexCount = vertexData.size();
		IndexCount = indexData.size();
	}
	MeshComponent::MeshComponent(const std::vector<MeshVertex>& vertexData)	{
		auto* renderer = VulkanRenderer::Get();

		VertexBuffer = renderer->CreateVertexBuffer(vertexData);
		VertexCount = vertexData.size();
	}
	MeshComponent::~MeshComponent() {
		auto* renderer = VulkanRenderer::Get();
		vmaDestroyBuffer(renderer->m_Allocator, IndexBuffer.Buffer, IndexBuffer.Allocation);
		vmaDestroyBuffer(renderer->m_Allocator, VertexBuffer.Buffer, VertexBuffer.Allocation);
	}
	void MeshComponent::Draw(VkCommandBuffer commandBuffer) {
		VkDeviceSize offset{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &VertexBuffer.Buffer, &offset);
		if (IndexBuffer.Buffer != VK_NULL_HANDLE) {
			vkCmdBindIndexBuffer(commandBuffer, IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, IndexCount,
				1, 0, 0, 0);
		} else {
			vkCmdDraw(commandBuffer, VertexCount, 1, 0, 0);
		}
	}
	CameraComponent::CameraComponent(glm::vec3 position) : Position{ position } {
		Up = glm::vec3(0.f, -1.f, 0.f);
		Forward = glm::vec3(0.f, 0.f, 1.f);
	}
	glm::mat4 CameraComponent::GetVP(float fov, float aspect, float near, float far) {
		glm::mat4 view = glm::lookAt(Position, Position + Forward, Up);
		glm::mat4 projection = glm::perspective(fov, aspect, near, far);
		return projection * view;
	}
	void CameraComponent::MoveForward(float amount)	{
		Position += Forward * amount;
	}
	void CameraComponent::MoveRight(float amount) {
		glm::vec3 right = glm::normalize(glm::cross(Forward, Up));
		Position += right * amount;
	}
	void CameraComponent::Pitch(float amount) {
		if (amount == 0.f) return;
		glm::vec3 right = glm::normalize(glm::cross(Forward, Up));
		glm::mat4 rotation = glm::rotate(amount, right);
		Forward = glm::normalize(glm::vec3(rotation * glm::vec4(Forward, 1.f)));
	}
	void CameraComponent::Yaw(float amount) {
		if (amount == 0.f) return;
		glm::mat4 rotation = glm::rotate(-amount, Up);
		Forward = glm::normalize(glm::vec3(rotation * glm::vec4(Forward, 1.f)));
	}
	TerrainComponent::TerrainComponent(const std::vector<MeshVertex>& vertexData) {
	}
}
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "Math.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vma/vk_mem_alloc.h>
#include "Vertex.h"

namespace FLOOF {

	struct MeshPushConstants {
		glm::mat4 mvp;
	};

	struct VulkanBuffer {
		VkBuffer Buffer = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocationInfo{};
	};

	struct VulkanImage {
		VkImage Image;
		VmaAllocation Allocation;
		VmaAllocationInfo AllocationInfo;
	};

	struct VulkanCombinedTextureSampler {
		VkImage Image = VK_NULL_HANDLE;
		VkImageView ImageView = VK_NULL_HANDLE;
		VkSampler Sampler = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocationInfo{};
	};

	class VulkanRenderer {
		friend class TextureComponent;
		friend class MeshComponent;
	public:
		VulkanRenderer(GLFWwindow* window);
		~VulkanRenderer();

		uint32_t GetNextSwapchainImage();
		VkExtent2D GetExtent() { return m_SwapChainExtent; }
		VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }

		VkCommandBuffer StartRecording(uint32_t imageIndex);
		void EndRecording();
		void SubmitAndPresent(uint32_t imageIndex);

		void FinishAllFrames();

		static VulkanRenderer* Get() { return s_Singleton; }
		VulkanBuffer CreateVertexBuffer(const std::vector<Vertex>& vertices);
		VulkanBuffer CreateIndexBuffer(const std::vector<uint32_t>& indices);
		void DestroyVulkanBuffer(VulkanBuffer* buffer);
	private:
		inline static VulkanRenderer* s_Singleton = nullptr;

		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t sizeX, uint32_t sizeY);

		GLFWwindow* m_Window;

		void InitSurface();
		void InitInstance();
		void InitDevice();

		void InitPhysicalDevice();
		void InitLogicalDevice();

		void InitVulkanAllocator();

		void InitSwapChain();
		void InitImageViews();

		void InitRenderPass();
		void InitGraphicsPipeline();

		void InitFramebuffers();
		void InitDepthBuffer();
		void InitCommandPool();
		void InitCommandBuffer();

		void InitDescriptorPools();

		void InitSyncObjects();

		void InitGlfwCallbacks();

		void CleanupSwapChain();
	public:
		void RecreateSwapChain();
	private:
		void WaitWhileMinimized();

		VkShaderModule MakeShaderModule(const char* path);

		VkImageViewCreateInfo MakeImageViewCreateInfo(int i);
		VkSwapchainCreateInfoKHR MakeSwapchainCreateInfo();

		void ValidatePhysicalDeviceExtentions();
		void ValidatePhysicalDeviceSurfaceCapabilities();

		VkSurfaceFormatKHR GetSurfaceFormat(VkFormat format, VkColorSpaceKHR colorSpace);
		VkPresentModeKHR GetPresentMode(VkPresentModeKHR presentMode);
		VkExtent2D GetWindowExtent();

		VkFormat FindDepthFormat();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features);

		VkFormat m_DepthFormat;
		VulkanImage m_DepthBuffer;
		VkImageView m_DepthBufferImageView;

		VkSurfaceKHR m_Surface;
		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_LogicalDevice;
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
		VmaAllocator m_Allocator;

		VkDescriptorSetLayout m_DescriptorSetLayout;

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		std::vector<VkImage> m_SwapChainImages;
		VkSurfaceFormatKHR m_SwapChainImageFormat;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_SwapChainExtent;

		VkRenderPass m_RenderPass;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;

	public:
		VkDescriptorSet AllocateTextureDescriptorSet();
		void FreeTextureDescriptorSet(VkDescriptorSet desctriptorSet);
	private:
		VkDescriptorPool m_TextureDescriptorPool;

		std::vector<VkImageView> m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t m_CurrentFrame = 0;

		const std::vector<const char*> m_RequiredDeviceExtentions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const std::vector<const char*> m_RequiredInstanceExtentions = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		struct QueueFamilyIndices {
			int Graphics = -1;
			int Compute = -1;
			int Transfer = -1;
			int SparseBinding = -1;
			int PresentIndex = -1;
		};
		QueueFamilyIndices m_QueueFamilyIndices{};
		void PopulateQueueFamilyIndices(QueueFamilyIndices& QFI);

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
		SwapChainSupportDetails m_SwapChainSupport;

		// Validation layers for debug builds.
	#ifndef NDEBUG
		const std::vector<const char*> m_ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
		};
	#endif
	};	
}
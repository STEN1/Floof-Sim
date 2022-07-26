#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>
#include <array>

#include <glm/glm.hpp>

namespace FLOOF {

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		return attributeDescriptions;
	}
};

const std::vector<Vertex> vertices = {
	{{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}}
};

class VulkanRenderer {
public:
	VulkanRenderer(GLFWwindow* window);
	~VulkanRenderer();

	void Draw();
	void Finish();
private:

	GLFWwindow* m_Window;

	void InitSurface();
	void InitInstance();
	void InitDevice();

	void InitPhysicalDevice();
	void InitLogicalDevice();

	void InitSwapChain();
	void InitImageViews();

	void InitRenderPass();
	void InitGraphicsPipeline();

	void InitFramebuffers();
	void InitCommandPool();
	void InitCommandBuffer();

	void InitSyncObjects();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void CleanupSwapChain();
	void RecreateSwapChain();
	void WaitWhileMinimized();

	VkShaderModule MakeShaderModule(const char* path);

	VkImageViewCreateInfo MakeImageViewCreateInfo(int i);
	VkSwapchainCreateInfoKHR MakeSwapchainCreateInfo();

	void ValidatePhysicalDeviceExtentions();
	void ValidatePhysicalDeviceSurfaceCapabilities();

	VkSurfaceFormatKHR GetSurfaceFormat(VkFormat format, VkColorSpaceKHR colorSpace);
	VkPresentModeKHR GetPresentMode(VkPresentModeKHR presentMode);
	VkExtent2D GetWindowExtent();

	VkSurfaceKHR m_Surface;
	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_LogicalDevice;
	VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;

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
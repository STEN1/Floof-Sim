#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>


namespace FLOOF {
class VulkanRenderer {
public:
	VulkanRenderer(GLFWwindow* window);
	~VulkanRenderer();
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

	std::vector<VkImageView> m_SwapChainImageViews;

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
	QueueFamilyIndices m_QFIndices{};
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
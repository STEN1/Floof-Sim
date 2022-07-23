#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace FLOOF {
class VulkanRenderer {
public:
	VulkanRenderer();
	~VulkanRenderer();
private:
	void InitInstance();
	void InitDevice();
	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_LogicalDevice;
	VkQueue m_GraphicsQueue;
	struct QueueFamilyIndices {
		int Graphics = -1;
		int Compute = -1;
		int Transfer = -1;
		int SparseBinding = -1;
	};
	QueueFamilyIndices m_QueueFamilyIndices{};
	// Validation layers for debug builds.
#ifndef NDEBUG
	const std::vector<const char*> m_ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
#endif
};	
}
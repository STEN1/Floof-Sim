#pragma once
#include <vulkan/vulkan.h>

namespace FLOOF {
class VulkanRenderer {
public:
	VulkanRenderer();
	~VulkanRenderer();
private:
	void CreateInstance();
	VkInstance m_Instance;
#ifdef NDEBUG
	const bool m_EnableValidationLayers = false;
#else
	const bool m_EnableValidationLayers = true;
#endif
};	
}
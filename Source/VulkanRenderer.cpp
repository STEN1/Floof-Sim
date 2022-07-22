#include "VulkanRenderer.h"
#include "Floof.h"
#include <GLFW/glfw3.h>
#include <vector>

namespace FLOOF {
VulkanRenderer::VulkanRenderer() {
	CreateInstance();
}
VulkanRenderer::~VulkanRenderer() {
    vkDestroyInstance(m_Instance, nullptr);
}
void VulkanRenderer::CreateInstance() {
	VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Floof";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    VkResult createInstanceResult = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    ASSERT(createInstanceResult == VK_SUCCESS, "Create instance failed with code: {}", (int)createInstanceResult);

    uint32_t extensionCount{};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    LOG("Available extensions:\n");
    for (const auto& extension : extensions) {
        LOG("\t {}\n", extension.extensionName);
    }
}
}

#include "VulkanRenderer.h"
#include "Floof.h"
#include <GLFW/glfw3.h>
#include <vector>

namespace FLOOF {
VulkanRenderer::VulkanRenderer() {
    InitInstance();
    InitDevice();
}
VulkanRenderer::~VulkanRenderer() {
    vkDestroyDevice(m_LogicalDevice, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}
void VulkanRenderer::InitInstance() {
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

    // Enable validation layers in debug builds.
    // TODO: Make custom callback function for validation layer logging.
#ifdef NDEBUG
    createInfo.enabledLayerCount = 0;
#else
    createInfo.enabledLayerCount = (uint32_t)m_ValidationLayers.size();
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
#endif

    VkResult createInstanceResult = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    ASSERT(createInstanceResult == VK_SUCCESS, "Create instance failed with code: {}", (int)createInstanceResult);

    uint32_t extensionCount{};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    LOG("Available extensions:\n");
    for (auto& extension : extensions) {
        LOG("\t{}\n", extension.extensionName);
    }
}
void VulkanRenderer::InitDevice() {
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
    LOG("Available devices:\n");
    for (auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        LOG("\t{}\n", deviceProperties.deviceName);
    }
    // Simply select first device.
    // Might want to select d-gpu for laptops.
    ASSERT(deviceCount > 0, "Device count: {}", deviceCount);
    m_PhysicalDevice = devices[0];

    uint32_t queueFamilyCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> qfp(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, qfp.data());
    for (int i = 0; i < qfp.size(); i++) {
        if (qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_QueueFamilyIndices.Graphics = i;
        }
        if (qfp[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            m_QueueFamilyIndices.Compute = i;
        }
        if (qfp[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            m_QueueFamilyIndices.Transfer = i;
        }
        if (qfp[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            m_QueueFamilyIndices.SparseBinding = i;
        }
    }

    VkDeviceQueueCreateInfo dqCreateInfo{};
    dqCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
    dqCreateInfo.queueCount = 1;
    float queuePrio = 1.f;
    dqCreateInfo.pQueuePriorities = &queuePrio;

    VkPhysicalDeviceFeatures deviceFeatures{};
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &deviceFeatures);

    VkDeviceCreateInfo dCreateInfo{};
    dCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dCreateInfo.pQueueCreateInfos = &dqCreateInfo;
    dCreateInfo.queueCreateInfoCount = 1;
    dCreateInfo.pEnabledFeatures = &deviceFeatures;
    dCreateInfo.enabledExtensionCount = 0;
    dCreateInfo.enabledLayerCount = 0;

    VkResult deviceCreationResult = vkCreateDevice(m_PhysicalDevice, &dCreateInfo, nullptr, &m_LogicalDevice);
    ASSERT(deviceCreationResult == VK_SUCCESS, "Failed to create device: {}", (int)deviceCreationResult);
    vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);
}
}

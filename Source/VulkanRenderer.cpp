#include "VulkanRenderer.h"
#include "Floof.h"

#undef max
#include <limits>
#include <algorithm>

namespace FLOOF {
VulkanRenderer::VulkanRenderer(GLFWwindow* window)
    : m_Window{ window } {
    InitInstance();
    InitSurface();
    InitDevice();
    InitSwapChain();
    InitImageViews();
}

VulkanRenderer::~VulkanRenderer() {
    for (auto& imageView : m_SwapChainImageViews) {
        vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);
    vkDestroyDevice(m_LogicalDevice, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanRenderer::InitSurface() {
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(m_Window);
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface);
    ASSERT(result == VK_SUCCESS, "Failed to create surface.");
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
    InitPhysicalDevice();
    InitLogicalDevice();
}

void VulkanRenderer::InitPhysicalDevice() {
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
    // Simply select first device. Then validate that it works.
    // Might want to select d-gpu for laptops.
    ASSERT(deviceCount > 0, "Device count: {}", deviceCount);
    m_PhysicalDevice = devices[0];

    PopulateQueueFamilyIndices(m_QFIndices);
    ValidatePhysicalDeviceExtentions();
    ValidatePhysicalDeviceSurfaceCapabilities();
}

void VulkanRenderer::InitLogicalDevice() {
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);
    VkDeviceQueueCreateInfo dqCreateInfo{};
    dqCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqCreateInfo.queueFamilyIndex = m_QFIndices.Graphics;
    dqCreateInfo.queueCount = 1;
    float queuePrio = 1.f;
    dqCreateInfo.pQueuePriorities = &queuePrio;

    VkDeviceCreateInfo dCreateInfo{};
    dCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dCreateInfo.pQueueCreateInfos = &dqCreateInfo;
    dCreateInfo.queueCreateInfoCount = 1;
    dCreateInfo.pEnabledFeatures = &m_PhysicalDeviceFeatures;
    dCreateInfo.enabledExtensionCount = m_RequiredDeviceExtentions.size();
    dCreateInfo.ppEnabledExtensionNames = m_RequiredDeviceExtentions.data();
    dCreateInfo.enabledLayerCount = 0;

    VkResult deviceCreationResult = vkCreateDevice(m_PhysicalDevice, &dCreateInfo, nullptr, &m_LogicalDevice);
    ASSERT(deviceCreationResult == VK_SUCCESS, "Failed to create device: {}", (int)deviceCreationResult);
    ASSERT(m_QFIndices.Graphics != -1, "Could not find queue with graphics support.");
    vkGetDeviceQueue(m_LogicalDevice, m_QFIndices.Graphics, 0, &m_GraphicsQueue);
    ASSERT(m_QFIndices.PresentIndex != -1, "Could not find queue with present support.");
    vkGetDeviceQueue(m_LogicalDevice, m_QFIndices.PresentIndex, 0, &m_PresentQueue);
}

void VulkanRenderer::InitSwapChain() {
    auto createInfo = MakeSwapchainCreateInfo();

    VkResult result = vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain);
    ASSERT(result == VK_SUCCESS, "Cant create swapchain.");

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());
}

void VulkanRenderer::InitImageViews() {
    m_SwapChainImageViews.resize(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
        auto createInfo = MakeImageViewCreateInfo(i);
        VkResult result = vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]);
        ASSERT(result == VK_SUCCESS, "Failed to create image view.");
    }
}

VkImageViewCreateInfo VulkanRenderer::MakeImageViewCreateInfo(int SwapChainImageIndex) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_SwapChainImages[SwapChainImageIndex];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_SwapChainImageFormat.format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    return createInfo;
}

VkSwapchainCreateInfoKHR VulkanRenderer::MakeSwapchainCreateInfo() {
    m_SwapChainImageFormat = GetSurfaceFormat(VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    m_PresentMode = GetPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
    m_SwapChainExtent = GetWindowExtent();

    uint32_t imageCount = m_SwapChainSupport.capabilities.minImageCount + 1;
    if (m_SwapChainSupport.capabilities.maxImageCount > 0 && imageCount > m_SwapChainSupport.capabilities.maxImageCount) {
        imageCount = m_SwapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_SwapChainImageFormat.format;
    createInfo.imageColorSpace = m_SwapChainImageFormat.colorSpace;
    createInfo.imageExtent = m_SwapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { m_QFIndices.Graphics, m_QFIndices.PresentIndex };
    if (m_QFIndices.Graphics != m_QFIndices.PresentIndex) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
    createInfo.presentMode = m_PresentMode;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = m_SwapChain;
    return createInfo;
}

void VulkanRenderer::ValidatePhysicalDeviceExtentions() {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

    uint32_t requiredExtentionsFound{};
    for (auto& availableEx : availableExtensions) {
        for (auto requiredEx : m_RequiredDeviceExtentions) {
            if (strcmp(availableEx.extensionName, requiredEx) == 0) {
                requiredExtentionsFound++;
            }
        }
    }
    ASSERT(requiredExtentionsFound == m_RequiredDeviceExtentions.size(), "All required device extentions not found.");
}

void VulkanRenderer::ValidatePhysicalDeviceSurfaceCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &m_SwapChainSupport.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
    if (formatCount != 0) {
        m_SwapChainSupport.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, m_SwapChainSupport.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        m_SwapChainSupport.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, m_SwapChainSupport.presentModes.data());
    }

    bool swapChainAdequate = false;
    swapChainAdequate = !m_SwapChainSupport.formats.empty() && !m_SwapChainSupport.presentModes.empty();
    ASSERT(swapChainAdequate, "Swap chain has no formats or present modes");
}

VkSurfaceFormatKHR VulkanRenderer::GetSurfaceFormat(VkFormat format, VkColorSpaceKHR colorSpace) {
    for (const auto& availableFormat : m_SwapChainSupport.formats) {
        if (availableFormat.format == format && availableFormat.colorSpace == colorSpace) {
            return availableFormat;
        }
    }
    return m_SwapChainSupport.formats[0];
}

VkPresentModeKHR VulkanRenderer::GetPresentMode(VkPresentModeKHR presentMode) {
    for (const auto& availablePresentMode : m_SwapChainSupport.presentModes) {
        if (availablePresentMode == presentMode) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::GetWindowExtent() {
    if (m_SwapChainSupport.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return m_SwapChainSupport.capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
            m_SwapChainSupport.capabilities.minImageExtent.width,
            m_SwapChainSupport.capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
            m_SwapChainSupport.capabilities.minImageExtent.height,
            m_SwapChainSupport.capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanRenderer::PopulateQueueFamilyIndices(QueueFamilyIndices& QFI) {
    uint32_t queueFamilyCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> qfp(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, qfp.data());
    VkBool32 presentSupport = false;
    for (int i = 0; i < qfp.size(); i++) {
        if (qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            QFI.Graphics = i;
        }
        if (qfp[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            QFI.Compute = i;
        }
        if (qfp[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            QFI.Transfer = i;
        }
        if (qfp[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            QFI.SparseBinding = i;
        }
        vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentSupport);
        if (presentSupport && QFI.PresentIndex == -1) {
            QFI.PresentIndex = i;
        }
    }
}
}

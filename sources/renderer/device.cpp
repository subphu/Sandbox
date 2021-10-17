//  Copyright © 2021 Subph. All rights reserved.
//

#include "device.hpp"

Device::Device() { }
Device::~Device() { }

void Device::cleanup() {
    LOG("Device::cleanup");
    m_cleaner.flush();
}

void Device::setup() {
    LOG("Device::setup");
    USE_FUNC(DebugCallback);
    
    VkApplicationInfo appInfo{};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = "Application";
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "No Engine";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_2;
    
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = DebugCallback;
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.multiViewport     = VK_TRUE;
    
    VECTOR<const char*> instanceExtensions = GetGLFWInstanceExtensions();
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VECTOR<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VECTOR<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    bool result = CheckLayerSupport(validationLayers);
    CHECK_BOOL(result, "validation layers requested, but not available!");
    
    m_appInfo             = appInfo;
    m_debugInfo           = debugInfo;
    m_deviceFeatures      = deviceFeatures;
    m_vDeviceExtensions   = deviceExtensions;
    m_vValidationLayers   = validationLayers;
    m_vInstanceExtensions = instanceExtensions;
}

void Device::createInstance() {
    LOG("Device::createInstance");
    VkApplicationInfo                  appInfo   = m_appInfo;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = m_debugInfo;
    VECTOR<const char*> validationLayers   = m_vValidationLayers;
    VECTOR<const char*> instanceExtensions = m_vInstanceExtensions;
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = UINT32(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceInfo.enabledLayerCount       = UINT32(validationLayers.size());
    instanceInfo.ppEnabledLayerNames     = validationLayers.data();
    instanceInfo.pNext                   = &debugInfo;
 
    VkInstance instance;
    VkResult   result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    CHECK_VKRESULT(result, "failed to create vulkan instance!");
    
    m_instance = instance;
    m_cleaner.push([=](){ vkDestroyInstance(m_instance, nullptr); });
}

void Device::createDebugMessenger() {
    LOG("Device::createDebugMessenger");
    VkResult result = CreateDebugUtilsMessengerEXT(m_instance, &m_debugInfo, nullptr, &m_debugMessenger);
    CHECK_VKRESULT(result, "failed to set up debug messenger!");
    m_cleaner.push([=](){ DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr); });
}

void Device::createSurface(GLFWwindow* window) {
    LOG("Device::createSurface");
    VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
    CHECK_VKRESULT(result, "failed to create window surface!");
}

void Device::selectPhysicalDevice() {
    LOG("Device::selectPhysicalDevice");
    VkInstance   instance = m_instance;
    VkSurfaceKHR surface  = m_surface;
    VkPhysicalDeviceFeatures deviceFeatures   = m_deviceFeatures;
    VECTOR<const char*>      deviceExtensions = m_vDeviceExtensions;
    
    uint32_t graphicQueueIndex = 0;
    uint32_t presentQueueIndex = 0;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    
    VECTOR<VkSurfaceFormatKHR> formats;
    VECTOR<VkPresentModeKHR>   modes;
    VECTOR<VkPhysicalDevice>   physicalDevices  = GetPhysicalDevices(instance);
    CHECK_ZERO(physicalDevices.size(), "failed to find GPUs with Vulkan support!");
    
    for (const auto& tempDevice : physicalDevices) {
        vkGetPhysicalDeviceProperties(tempDevice, &m_deviceProperties);
        LOG(m_deviceProperties.deviceName);

        physicalDevice    = tempDevice;
        presentQueueIndex = FindPresentQueueIndex(tempDevice, surface);
        graphicQueueIndex = FindGraphicQueueIndex(tempDevice);
        
        formats = GetSurfaceFormatKHR(tempDevice, surface);
        modes   = GetPresentModeKHR  (tempDevice, surface);
        
        bool swapchainAdequate  = !formats.empty() && !modes.empty();
        bool hasFamilyIndex     = graphicQueueIndex > -1 && presentQueueIndex > -1;
        bool featureSupported   = CheckFeatureSupport(tempDevice, deviceFeatures);
        bool extensionSupported = CheckDeviceExtensionSupport(tempDevice, deviceExtensions);
        
        if (swapchainAdequate && hasFamilyIndex && extensionSupported && featureSupported) break;
    }
    
    m_surfaceFormat     = FindSufraceFormat(formats);
    m_presentMode       = FindPresentMode(modes);
    m_physicalDevice    = physicalDevice;
    m_graphicQueueIndex = graphicQueueIndex;
    m_presentQueueIndex = presentQueueIndex;
}

void Device::createLogicalDevice() {
    LOG("Device::createLogicalDevice");
    VkPhysicalDevice         physicalDevice = m_physicalDevice;
    VkPhysicalDeviceFeatures deviceFeatures = m_deviceFeatures;
    VECTOR<const char*> deviceExtensions  = m_vDeviceExtensions;
    VECTOR<const char*> validationLayers  = m_vValidationLayers;
    uint32_t graphicQueueIndex = m_graphicQueueIndex;
    uint32_t presentQueueIndex = m_presentQueueIndex;
    
    float queuePriority = 1.f;
    VECTOR<VkDeviceQueueCreateInfo> queueInfos;
    for (uint32_t familyIndex : {graphicQueueIndex, presentQueueIndex}) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex  = familyIndex;
        queueInfo.queueCount        = 1;
        queueInfo.pQueuePriorities  = &queuePriority;
        queueInfos.push_back(queueInfo);
    }
    
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount     = UINT32(queueInfos.size());
    deviceInfo.pQueueCreateInfos        = queueInfos.data();
    deviceInfo.pEnabledFeatures         = &deviceFeatures;
    deviceInfo.enabledExtensionCount    = UINT32(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames  = deviceExtensions.data();
    deviceInfo.enabledLayerCount        = UINT32(validationLayers.size());
    deviceInfo.ppEnabledLayerNames      = validationLayers.data();
    
    VkDevice device;
    VkResult result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
    CHECK_VKRESULT(result, "failed to create logical device");
    
    m_device = device;
    vkGetDeviceQueue(device, graphicQueueIndex, 0, &m_graphicQueue);
    vkGetDeviceQueue(device, presentQueueIndex, 0, &m_presentQueue);
    m_cleaner.push([=](){ vkDestroyDevice(m_device, nullptr); });
}

uint32_t Device::findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags flags) {
    VkPhysicalDevice physicalDevice = m_physicalDevice;
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);
    
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            (properties.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    
    throw std::runtime_error("failed to find suitable memory type!");
}

VkInstance         Device::getInstance()       { return m_instance; }
VkSurfaceKHR       Device::getSurface()        { return m_surface; }
VkPhysicalDevice   Device::getPhysicalDevice() { return m_physicalDevice; }
VkDevice           Device::getDevice()         { return m_device; }

VkQueue            Device::getGraphicQueue()   { return m_graphicQueue; }
VkSurfaceFormatKHR Device::getSurfaceFormat()  { return m_surfaceFormat; }
VkPresentModeKHR   Device::getPresentMode()    { return m_presentMode;}

uint32_t Device::getGraphicQueueIndex() { return m_graphicQueueIndex; }
uint32_t Device::getPresentQueueIndex() { return m_presentQueueIndex; }


// Private ==================================================


VkSurfaceFormatKHR Device::FindSufraceFormat(VECTOR<VkSurfaceFormatKHR> availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format     == VK_FORMAT_B8G8R8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR Device::FindPresentMode(VECTOR<VkPresentModeKHR> availableModes) {
    for (const auto& availableMode : availableModes) {
        if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return availableMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VECTOR<const char*> Device::GetGLFWInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    VECTOR<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

VECTOR<VkPhysicalDevice> Device::GetPhysicalDevices(VkInstance instance) {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    VECTOR<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());
    return devices;
}

VECTOR<VkSurfaceFormatKHR> Device::GetSurfaceFormatKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    VECTOR<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
    return formats;
}

VECTOR<VkPresentModeKHR> Device::GetPresentModeKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    VECTOR<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, modes.data());
    return modes;
}

VECTOR<VkQueueFamilyProperties> Device::GetQueueFamilyProperties(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    VECTOR<VkQueueFamilyProperties> properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, properties.data());
    return properties;
}

int Device::FindGraphicQueueIndex(VkPhysicalDevice physicalDevice) {
    VECTOR<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) return i;
    }
    return -1;
}

int Device::FindPresentQueueIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VECTOR<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) return i;
    }
    return -1;
}

bool Device::CheckLayerSupport(VECTOR<const char*> layers) {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    VECTOR<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());
    
    std::set<std::string> requiredLayers(layers.begin(), layers.end());
    for (const auto& layerProperties : availableLayers) {
        requiredLayers.erase(layerProperties.layerName);
    }
    return requiredLayers.empty();
}

bool Device::CheckFeatureSupport(VkPhysicalDevice device, VkPhysicalDeviceFeatures features) {
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
    uint size = sizeof(VkPhysicalDeviceFeatures)/sizeof(VkBool32);
    VkBool32* pBoolSupported = reinterpret_cast<VkBool32*>(&supportedFeatures);
    VkBool32* pBoolFeatures  = reinterpret_cast<VkBool32*>(&features);
    
    bool supported = true;
    for (int i = 0; i < size; i++) {
        supported &= pBoolFeatures[i] ? pBoolSupported[i] : supported;
    }
    return supported;
}


bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, VECTOR<const char*> extensions) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    VECTOR<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, availableExtensions.data());

    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

VkResult Device::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Device::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Device::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "Validation layer: \n" << pCallbackData->pMessage << std::endl << std::endl;
    return VK_FALSE;
}

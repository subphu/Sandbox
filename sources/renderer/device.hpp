//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"

class Device {
    
public:
    Device();
    ~Device();
    
    void cleanup();
    
    void setup();
    void createInstance();
    void createSurface(GLFWwindow* m_window);
    void createDebugMessenger();
    void selectPhysicalDevice();
    void createLogicalDevice();
    void waitIdle();
    void waitAllQueueIdle();
    
    VkInstance         getInstance();
    VkSurfaceKHR       getSurface();
    VkPhysicalDevice   getPhysicalDevice();
    VkDevice           getDevice();
    
    VkQueue            getGraphicQueue();
    VkQueue            getPresentQueue();
    VkSurfaceFormatKHR getSurfaceFormat();
    VkPresentModeKHR   getPresentMode();
    
    uint32_t getGraphicQueueIndex();
    uint32_t getPresentQueueIndex();
    uint32_t findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
    
    VkApplicationInfo m_appInfo{};
    VkDebugUtilsMessengerCreateInfoEXT m_debugInfo{};
    VkPhysicalDeviceFeatures m_deviceFeatures{};
    
private:
    Cleaner m_cleaner;
    
    VECTOR<const char*> m_vInstanceExtensions{};
    VECTOR<const char*> m_vDeviceExtensions{};
    VECTOR<const char*> m_vValidationLayers{};
    
    VkInstance       m_instance;
    VkSurfaceKHR     m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice         m_device;
    
    VkPhysicalDeviceProperties m_deviceProperties{};
    VkDebugUtilsMessengerEXT   m_debugMessenger;

    VkSurfaceFormatKHR m_surfaceFormat{};
    VkPresentModeKHR   m_presentMode  {};
    
    uint32_t m_graphicQueueIndex = 0;
    uint32_t m_presentQueueIndex = 0;

    VkQueue m_graphicQueue;
    VkQueue m_presentQueue;

    static VkSurfaceFormatKHR FindSufraceFormat(VECTOR<VkSurfaceFormatKHR> surfaceFormats);
    static VkPresentModeKHR   FindPresentMode  (VECTOR<VkPresentModeKHR>   presentModes);
    
    static VECTOR<const char*>             GetGLFWInstanceExtensions();
    static VECTOR<VkPhysicalDevice>        GetPhysicalDevices(VkInstance instance);
    static VECTOR<VkSurfaceFormatKHR>      GetSurfaceFormatKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static VECTOR<VkPresentModeKHR>        GetPresentModeKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static VECTOR<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice);
    
    static int FindGraphicQueueIndex(VkPhysicalDevice physicalDevice);
    static int FindPresentQueueIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    
    static bool CheckLayerSupport(VECTOR<const char*> layers);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, VECTOR<const char*> extensions);
    static bool CheckFeatureSupport(VkPhysicalDevice device, VkPhysicalDeviceFeatures features);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData);
};


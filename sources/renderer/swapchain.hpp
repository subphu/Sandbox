//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "renderpass.hpp"
#include "frame.hpp"
#include "device.hpp"

class Swapchain {
    
public:
    ~Swapchain();
    Swapchain();
    
    void cleanup();
    void recreate();
    
    void setup();
    void create();
    void createRenderpass();
    void createFrames(Renderpass* renderpass);
    
    void prepareFrame();
    void submitFrame();
    void presentFrame();
    
    VkFence getSubmitFence();
    VkCommandBuffer getCommandBuffer();
    VkSemaphore getImageSemaphore();
    VkSemaphore getSubmitSemaphore();
    Frame* getCurrentFrame();
    
    VkSwapchainCreateInfoKHR m_swapchainInfo{};
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Renderpass* m_pRenderpass;
    
    uint m_totalFrame = 0;
    uint m_frameIdx = 0;
    uint m_semaphoreIdx = 0;
    
    VECTOR<Frame*>  m_frames;
    VECTOR<VkFence> m_submitFences;
    VECTOR<VkSemaphore> m_submitSemaphores;
    VECTOR<VkSemaphore> m_imageSemaphores;
    VECTOR<VkCommandBuffer> m_commandBuffers;
    
    VkSwapchainKHR m_swapchain;
    
    void checkSwapchainResult(VkResult result);
    static VECTOR<VkImage> GetSwapchainImages(VkSwapchainKHR swapchain);
};

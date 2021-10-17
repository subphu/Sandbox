//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "device.hpp"

class Commander {
    
public:
    ~Commander();
    Commander();
    
    
    void cleanup();
    
    void setupPool();
    void createPool();
    
    VkCommandBuffer              createCommandBuffer();
    std::vector<VkCommandBuffer> createCommandBuffers(uint32_t count);
    
    void beginSingleTimeCommands(VkCommandBuffer commandBuffer);
    void endSingleTimeCommands  (VkCommandBuffer commandBuffer);
    
    VkCommandPoolCreateInfo m_poolInfo{};
    
private:
    Device* m_pDevice;
    Cleaner m_cleaner;
    
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    
    
};



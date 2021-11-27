//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "device.hpp"

class Buffer {
    
public:
    ~Buffer();
    Buffer();

    void cleanup();
    
    void setup (VkDeviceSize size, VkBufferUsageFlags usage);
    void create();
    
    void createBuffer();
    void allocateBufferMemory();
    void createDescriptorInfo();
    
    void cmdCopyFromBuffer(VkBuffer sourceBuffer, VkDeviceSize size);
    
    void* fillBuffer    (const void* address, VkDeviceSize size, uint32_t shift = 0);
    void* fillBufferFull(const void* address);
    
    void* mapMemory  (VkDeviceSize size);
    void  unmapMemory();
    
    VkBuffer       getBuffer      ();
    VkDeviceSize   getBufferSize  ();
    VkDeviceMemory getBufferMemory();
    VkDescriptorBufferInfo* getDescriptorInfo();
    
    
    VkBufferCreateInfo m_bufferInfo{};

private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    
    unsigned char* m_desc;
    
    VkBuffer         m_buffer         = VK_NULL_HANDLE;
    VkDeviceMemory   m_bufferMemory   = VK_NULL_HANDLE;
    VkDescriptorBufferInfo m_descriptorInfo{};
    
    static VkBufferCreateInfo GetDefaultBufferCreateInfo();
};

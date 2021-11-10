//  Copyright © 2021 Subph. All rights reserved.
//

#include "buffer.hpp"

#include "../system.hpp"

Buffer::~Buffer() {}
Buffer::Buffer() : m_pDevice(System::Device()) {}

void Buffer::cleanup() {
    LOG("Buffer::cleanup");
    m_cleaner.flush();
}

void Buffer::setup(VkDeviceSize size, VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferInfo = GetDefaultBufferCreateInfo();
    
    bufferInfo.size  = size;
    bufferInfo.usage = usage;
    
    m_bufferInfo = bufferInfo;
}

void Buffer::create() {
    createBuffer();
    allocateBufferMemory();
}

void Buffer::createBuffer() {
    LOG("Buffer::createBuffer");
    VkDevice device = m_pDevice->getDevice();
    VkResult result = vkCreateBuffer(device, &m_bufferInfo, nullptr, &m_buffer);
    CHECK_VKRESULT(result, "failed to buffer!");
    m_cleaner.push([=](){ vkDestroyBuffer(device, m_buffer, nullptr); });
}

void Buffer::allocateBufferMemory() {
    LOG("Buffer::allocateBufferMemory");
    Device*  pDevice = m_pDevice;
    VkDevice device  = m_pDevice->getDevice();
    VkBuffer buffer  = m_buffer;
    
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
    
    uint32_t memoryTypeIndex = pDevice->findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkDeviceMemory bufferMemory;
    VkResult       result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    CHECK_VKRESULT(result, "failed to allocate buffer memory!");
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    
    m_bufferMemory = bufferMemory;
    m_cleaner.push([=](){ vkFreeMemory(device, m_bufferMemory, nullptr); });
}

void Buffer::cmdCopyFromBuffer(VkBuffer sourceBuffer, VkDeviceSize size) {
    LOG("Buffer::cmdCopyFromBuffer");
    VkBuffer   buffer    = m_buffer;
    Commander* pCommander = System::Commander();
    
    VkCommandBuffer commandBuffer = pCommander->createCommandBuffer();
    pCommander->beginSingleTimeCommands(commandBuffer);
    VkBufferCopy    copyRegion = { 0, 0, size };
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, buffer, 1, &copyRegion);
    pCommander->endSingleTimeCommands(commandBuffer);
}

void* Buffer::fillBuffer(const void* address, VkDeviceSize size, uint32_t shift) {
    void* ptr = mapMemory(size);
    ptr = static_cast<char*>(ptr) + shift;
    memcpy(ptr, address, size);
    unmapMemory();
    return ptr;
}

void* Buffer::fillBufferFull(const void* address) {
    return fillBuffer(address, static_cast<size_t>(m_bufferInfo.size));
}

void* Buffer::mapMemory(VkDeviceSize size) {
    void* ptr;
    VkDevice device = m_pDevice->getDevice();
    vkMapMemory(device, m_bufferMemory, 0, size, 0, &ptr);
    return ptr;
}

void Buffer::unmapMemory() {
    VkDevice device = m_pDevice->getDevice();
    vkUnmapMemory(device, m_bufferMemory);
}

VkBuffer       Buffer::getBuffer      () { return m_buffer;       }
VkDeviceMemory Buffer::getBufferMemory() { return m_bufferMemory; }
VkDeviceSize   Buffer::getBufferSize  () { return m_bufferInfo.size; }
VkDescriptorBufferInfo Buffer::getDescriptorInfo() {
    VkDescriptorBufferInfo descriptorInfo{};
    descriptorInfo.buffer = m_buffer;
    descriptorInfo.range  = m_bufferInfo.size;
    descriptorInfo.offset = 0;
    return descriptorInfo;
}


// Private ==================================================


VkBufferCreateInfo Buffer::GetDefaultBufferCreateInfo() {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return bufferInfo;
}

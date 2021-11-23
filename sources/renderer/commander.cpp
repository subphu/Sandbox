//  Copyright © 2021 Subph. All rights reserved.
//

#include "commander.hpp"

#include "system.hpp"

Commander::~Commander() {}
Commander::Commander() : m_pDevice(System::Device()) {}

void Commander::cleanup() { m_cleaner.flush("Commander"); }

void Commander::setupPool() {
    m_poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    m_poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    m_poolInfo.queueFamilyIndex = m_pDevice->getGraphicQueueIndex();
}

void Commander::createPool() {
    LOG("Commander::createPool");
    VkDevice device = m_pDevice->getDevice();
    VkResult result = vkCreateCommandPool(device, &m_poolInfo, nullptr, &m_commandPool);
    CHECK_VKRESULT(result, "failed to create command pool!");
    m_cleaner.push([=](){ vkDestroyCommandPool(device, m_commandPool, nullptr); });
}

VkCommandBuffer Commander::createCommandBuffer() {
    LOG("Commander::createCommandBuffer");
    VkDevice      device      = m_pDevice->getDevice();
    VkCommandPool commandPool = m_commandPool;
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    return commandBuffer;
}

std::vector<VkCommandBuffer> Commander::createCommandBuffers(uint32_t size) {
    LOG("Commander::createCommandBuffers");
    VkDevice      device      = m_pDevice->getDevice();
    VkCommandPool commandPool = m_commandPool;
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = size;
    
    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(size);
    
    vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    return commandBuffers;
}

void Commander::beginSingleTimeCommands(VkCommandBuffer commandBuffer) {
    LOG("Commander::beginSingleTimeCommands");
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void Commander::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    LOG("Commander::endSingleTimeCommands");
    VkDevice      device      = m_pDevice->getDevice();
    VkQueue       queue       = m_pDevice->getGraphicQueue();
    VkCommandPool commandPool = m_commandPool;
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;
    
    vkQueueSubmit  (queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

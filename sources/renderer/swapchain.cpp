//  Copyright © 2021 Subph. All rights reserved.
//

#include "swapchain.hpp"

#include "../system.hpp"

Swapchain::~Swapchain() {}
Swapchain::Swapchain() : m_pDevice(System::Device()) {}

void Swapchain::cleanup() { m_cleaner.flush("Swapchain"); }

void Swapchain::recreate() {
    LOG("Swapchain::recreate");
    cleanup();
    setup();
    create();
    createFrames(m_pRenderpass);
}

void Swapchain::setup() {
    VkSurfaceKHR             surface       = m_pDevice->getSurface();
    VkSurfaceFormatKHR       surfaceFormat = m_pDevice->getSurfaceFormat();
    VkPresentModeKHR         presentMode   = m_pDevice->getPresentMode();
    VkSurfaceCapabilitiesKHR capabilities  = m_pDevice->getSurfaceCapabilities();
    
    // graphicQueueIndex == presentQueueIndex
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface          = surface;
    swapchainInfo.preTransform     = capabilities.currentTransform;
    swapchainInfo.minImageCount    = capabilities.minImageCount + 1;
    swapchainInfo.imageExtent      = capabilities.currentExtent;
    swapchainInfo.imageFormat      = surfaceFormat.format;
    swapchainInfo.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode      = presentMode;
    swapchainInfo.clipped          = VK_TRUE;
    swapchainInfo.oldSwapchain     = VK_NULL_HANDLE;

    m_swapchainInfo = swapchainInfo;
}

void Swapchain::create() {
    LOG("Swapchain::create");
    VkDevice device = m_pDevice->getDevice();
    VkResult result = vkCreateSwapchainKHR(device, &m_swapchainInfo, nullptr, &m_swapchain);
    CHECK_VKRESULT(result, "failed to create swapchain!");
    m_cleaner.push([=](){ vkDestroySwapchainKHR(device, m_swapchain, nullptr); });
}

void Swapchain::createFrames(Renderpass* renderpass) {
    LOG("Swapchain::createFrames");
    VkDevice       device      = m_pDevice->getDevice();
    Commander*     pCommander  = System::Commander();
    Renderpass*    pRenderpass = renderpass;
    VkSwapchainKHR swapchain   = m_swapchain;
    VkSwapchainCreateInfoKHR swapchainInfo = m_swapchainInfo;

    VECTOR<VkImage> swapchainImages = GetSwapchainImages(swapchain);
    uint32_t totalFrame = UINT32(swapchainImages.size());
    uint32_t width  = swapchainInfo.imageExtent.width;
    uint32_t height = swapchainInfo.imageExtent.height;

    VECTOR<VkCommandBuffer> commandBuffers = pCommander->createCommandBuffers(totalFrame);
    VECTOR<VkSemaphore> imageSemaphores(totalFrame);
    VECTOR<VkSemaphore> submitSemaphores(totalFrame);
    VECTOR<VkFence>     fences(totalFrame);
    VECTOR<Frame*>      frames(totalFrame);
    
    VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < totalFrame; i++) {
        frames[i] = new Frame({width, height});
        frames[i]->createImageResource(swapchainImages[i], swapchainInfo.imageFormat);
        frames[i]->createFramebuffer(pRenderpass);
        
        vkCreateFence(device, &fenceInfo, nullptr, &fences[i]);
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageSemaphores[i]);
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &submitSemaphores[i]);

        m_cleaner.push([=](){ frames[i]->cleanup(); });
        m_cleaner.push([=](){ vkDestroyFence(device, fences[i], nullptr); });
        m_cleaner.push([=](){ vkDestroySemaphore(device, imageSemaphores[i], nullptr); });
        m_cleaner.push([=](){ vkDestroySemaphore(device, submitSemaphores[i], nullptr); });
    }
    m_cleaner.push([=]() { System::Device()->waitAllQueueIdle(); });
    
    m_frames = frames;
    m_totalFrame = totalFrame;
    m_submitFences = fences;
    m_submitSemaphores = submitSemaphores;
    m_imageSemaphores = imageSemaphores;
    m_commandBuffers = commandBuffers;
    m_pRenderpass = renderpass;
}

void Swapchain::prepareFrame() {
//    LOG("Swapchain::prepareFrame");
    VkDevice device = System::Device()->getDevice();
    VkResult result = vkAcquireNextImageKHR(device, m_swapchain,
                                            UINT64_MAX, getImageSemaphore(),
                                            VK_NULL_HANDLE, &m_frameIdx);

    checkSwapchainResult(result);
    
    VkFence submitFence = getSubmitFence();
    vkWaitForFences(device, 1, &submitFence, VK_TRUE, UINT64_MAX);
}

void Swapchain::submitFrame() {
//    LOG("Swapchain::submitFrame");
    VkDevice device = m_pDevice->getDevice();
    VkQueue graphicQueue = m_pDevice->getGraphicQueue();
    VkFence submitFence  = getSubmitFence();
    VkSemaphore imageSemaphore  = getImageSemaphore();
    VkSemaphore submitSemaphore = getSubmitSemaphore();
    VkCommandBuffer cmdBuffer   = getCommandBuffer();
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &imageSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &submitSemaphore;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &cmdBuffer;
    
    vkResetFences(device, 1, &submitFence);
    VkResult result = vkQueueSubmit(graphicQueue, 1, &submitInfo, submitFence);
    CHECK_VKRESULT(result, "failed to submit draw command buffer!");
}

void Swapchain::presentFrame() {
//    LOG("Swapchain::presentFrame");
    VkQueue        presentQueue = m_pDevice->getPresentQueue();
    VkSwapchainKHR swapchain = m_swapchain;
    VkSemaphore    submitSemaphore = getSubmitSemaphore();
    uint frameIdx = m_frameIdx;
    uint totalFrame = m_totalFrame;
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchain;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &submitSemaphore;
    presentInfo.pImageIndices      = &frameIdx;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    checkSwapchainResult(result);
    
    m_semaphoreIdx = (m_semaphoreIdx + 1) % totalFrame;
}

void Swapchain::checkSwapchainResult(VkResult result) {
    if (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR) { return; }
    LOG("swap chain failed!");
    recreate();
}

Frame* Swapchain::getCurrentFrame() { return m_frames[m_frameIdx]; }
VkFence Swapchain::getSubmitFence() { return m_submitFences[m_frameIdx]; }
VkCommandBuffer Swapchain::getCommandBuffer() { return m_commandBuffers[m_frameIdx]; }
VkSemaphore Swapchain::getImageSemaphore()  { return m_imageSemaphores[m_semaphoreIdx]; }
VkSemaphore Swapchain::getSubmitSemaphore() { return m_submitSemaphores[m_semaphoreIdx]; }


// Private ==================================================


VECTOR<VkImage> Swapchain::GetSwapchainImages(VkSwapchainKHR swapchain) {
    VkDevice device = System::Device()->getDevice();
    
    uint32_t count;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    std::vector<VkImage> swapchainImages(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapchainImages.data());
    return swapchainImages;
}


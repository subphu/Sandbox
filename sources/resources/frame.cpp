//  Copyright © 2021 Subph. All rights reserved.
//

#include "frame.hpp"

#include "../system.hpp"

Frame::~Frame() {}
Frame::Frame(UInt2D size) : m_pDevice(System::Device()), m_size(size) {}

void Frame::cleanup() {
    LOG("Frame::cleanup");
    m_cleaner.flush();
}

void Frame::createDepthResource() {
    m_pDepthImage = new Image();
    m_pDepthImage->setupForDepth(m_size);
    m_pDepthImage->create();
    m_attachments.push_back(m_pDepthImage->getImageView());
    m_cleaner.push([=](){ m_pDepthImage->cleanup(); });
}

void Frame::createImageResource() {
    m_pImage = new Image();
    m_pImage->setupForColor(m_size);
    m_pImage->create();
    m_attachments.push_back(m_pImage->getImageView());
    m_cleaner.push([=](){ m_pImage->cleanup(); });
}

void Frame::createImageResource(VkImage image, VkFormat format) {
    m_pImage = new Image();
    m_pImage->setupForSwapchain(image, format);
    m_pImage->createForSwapchain();
    m_attachments.push_back(m_pImage->getImageView());
    m_cleaner.push([=](){ m_pImage->cleanup(); });
}

void Frame::createFramebuffer(Renderpass* renderpass) {
    LOG("createFramebuffer");
    VkDevice device = m_pDevice->getDevice();
    UInt2D   size   = m_size;
    VECTOR<VkImageView> attachments = m_attachments;
    
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderpass->get();
    framebufferInfo.attachmentCount = UINT32(attachments.size());
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = size.width;
    framebufferInfo.height          = size.height;
    framebufferInfo.layers          = 1;
    
    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
    CHECK_VKRESULT(result, "failed to create framebuffer!");
    m_cleaner.push([=](){ vkDestroyFramebuffer(device, framebuffer, nullptr); });
    
    m_framebuffer = framebuffer;
}

VkFramebuffer Frame::getFramebuffer() { return m_framebuffer; }
UInt2D Frame::getExtent2D() { return m_size; }
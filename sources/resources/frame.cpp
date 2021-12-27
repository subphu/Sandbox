//  Copyright © 2021 Subph. All rights reserved.
//

#include "frame.hpp"

#include "../system.hpp"

Frame::~Frame() {}
Frame::Frame(UInt2D size) : m_pDevice(System::Device()), m_size(size) {}

void Frame::cleanup() { m_cleaner.flush("Frame"); }

void Frame::createDepthResource() {
    m_pDepthImage = new Image();
    m_pDepthImage->setupForDepth(m_size);
    m_pDepthImage->createWithSampler();
    m_attachments.push_back(m_pDepthImage->getImageView());
    m_cleaner.push([=](){ m_pDepthImage->cleanup(); });
    m_cleaner.push([=](){ m_attachments.pop_back(); });
}

void Frame::createImageResource() {
    m_pColorImage = new Image();
    m_pColorImage->setupForColor(m_size);
    m_pColorImage->createWithSampler();
    m_attachments.push_back(m_pColorImage->getImageView());
    m_cleaner.push([=](){ m_pColorImage->cleanup(); });
    m_cleaner.push([=](){ m_attachments.pop_back(); });
}

void Frame::createImageResource(VkImage image, VkFormat format) {
    m_pColorImage = new Image();
    m_pColorImage->setupForSwapchain(image, format);
    m_pColorImage->createForSwapchain();
    m_attachments.push_back(m_pColorImage->getImageView());
    m_cleaner.push([=](){ m_pColorImage->cleanup(); });
    m_cleaner.push([=](){ m_attachments.pop_back(); });
}

void Frame::createCubeResource() {
    m_layer = 6;
    m_pColorImage = new Image();
    m_pColorImage->setupForCubemap(m_size);
    m_pColorImage->createWithSampler();
    m_attachments.push_back(m_pColorImage->getImageView());
    m_cleaner.push([=](){ m_pColorImage->cleanup(); });
    m_cleaner.push([=](){ m_attachments.pop_back(); });
}

void Frame::createFramebuffer(Renderpass* renderpass) {
    LOG("createFramebuffer");
    VkDevice device = m_pDevice->getDevice();
    
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderpass->get();
    framebufferInfo.attachmentCount = UINT32(m_attachments.size());
    framebufferInfo.pAttachments    = m_attachments.data();
    framebufferInfo.width           = m_size.width;
    framebufferInfo.height          = m_size.height;
    framebufferInfo.layers          = m_layer;
    
    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
    CHECK_VKRESULT(result, "failed to create framebuffer!");
    m_cleaner.push([=](){ vkDestroyFramebuffer(device, framebuffer, nullptr); });
    
    m_framebuffer = framebuffer;
}

VkFramebuffer Frame::getFramebuffer() { return m_framebuffer; }
UInt2D Frame::getSize      () { return m_size; }
Image* Frame::getColorImage() { return m_pColorImage;}
Image* Frame::getDepthImage() { return m_pDepthImage;}

void Frame::setSize(UInt2D size) { m_size = size; }

//  Copyright © 2021 Subph. All rights reserved.
//

#include "renderpass.hpp"

#include "../system.hpp"

Renderpass::~Renderpass() {}
Renderpass::Renderpass() : m_pDevice(System::Device()) {}

void Renderpass::cleanup() {
    LOG("Renderpass::cleanup");
    m_cleaner.flush();
}

void Renderpass::setupColorAttachment(VkFormat format) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format          = format;
    colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    m_attachments.push_back(colorAttachment);
    
    m_colorAttachmentRef.attachment = 0;
    m_colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    m_subpass.colorAttachmentCount = 1;
    m_subpass.pColorAttachments = &m_colorAttachmentRef;
}

void Renderpass::setupDepthAttachment(VkFormat format) {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format          = format;
    depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    m_attachments.push_back(depthAttachment);
    
    m_depthAttachmentRef.attachment = 1;
    m_depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    m_subpass.pDepthStencilAttachment = &m_depthAttachmentRef;
}

void Renderpass::setup() {
    m_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    
    m_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    m_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_dependency.srcAccessMask = 0;
    m_dependency.dstSubpass    = 0;
    m_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    m_renderpassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    m_renderpassInfo.attachmentCount  = UINT32(m_attachments.size());
    m_renderpassInfo.pAttachments     = m_attachments.data();
    m_renderpassInfo.subpassCount     = 1;
    m_renderpassInfo.pSubpasses       = &m_subpass;
    m_renderpassInfo.dependencyCount  = 1;
    m_renderpassInfo.pDependencies    = &m_dependency;
}

void Renderpass::create() {
    VkDevice device = m_pDevice->getDevice();
    VkResult result = vkCreateRenderPass(device, &m_renderpassInfo, nullptr, &m_renderpass);
    CHECK_VKRESULT(result, "failed to create render pass!");
    m_cleaner.push([=](){ vkDestroyRenderPass(device, m_renderpass, nullptr); });
}

VkRenderPass Renderpass::get() { return m_renderpass; }

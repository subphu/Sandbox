//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "device.hpp"

class Renderpass {
    
public:
    ~Renderpass();
    Renderpass();
    
    void cleanup();
    void setupColorAttachment(VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    void setupDepthAttachment(VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT);
    void setup();
    void create();
    
    VkRenderPass get();
    
    VkRenderPassCreateInfo m_renderpassInfo{};
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    
    VkAttachmentReference m_colorAttachmentRef{};
    VkAttachmentReference m_depthAttachmentRef{};
    VkSubpassDescription m_subpass{};
    VkSubpassDependency m_dependency{};
    
    VECTOR<VkAttachmentDescription> m_attachments;
    
    VkRenderPass m_renderpass;
    
};

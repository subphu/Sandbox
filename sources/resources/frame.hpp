//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "device.hpp"
#include "renderpass.hpp"
#include "image.hpp"


class Frame {
    
public:
    ~Frame();
    Frame(UInt2D size);
    
    void cleanup();
    
    void createDepthResource();
    void createImageResource();
    void createImageResource(VkImage image, VkFormat format);
    void createCubeResource();
    void createFramebuffer(Renderpass* renderpass);
    
    VkFramebuffer getFramebuffer();
    UInt2D        getSize();
    Image*        getColorImage();
    Image*        getDepthImage();
    
    void setSize(UInt2D size);
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Image*  m_pColorImage;
    Image*  m_pDepthImage;
    
    int     m_layer = 1;
    UInt2D  m_size{};
    VECTOR<VkImageView> m_attachments;
    
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
};

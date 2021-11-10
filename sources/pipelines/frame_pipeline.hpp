//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/renderpass.hpp"
#include "../resources/frame.hpp"

class FramePipeline {
    
public:
    ~FramePipeline();
    FramePipeline();
    
    void cleanup();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    
    void setFrame(Frame* frame);
    
    Renderpass* getRenderpass();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Renderpass* m_pRenderpass;
    Frame* m_pFrame;
    
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
    
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
};

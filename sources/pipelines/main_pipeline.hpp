//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/renderpass.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/frame.hpp"

class MainPipeline {
    
public:
    ~MainPipeline();
    MainPipeline();
    
    void cleanup();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    void createFrame();
    
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Renderpass* m_pRenderpass;
    Descriptor* m_pDescriptor;
    
    Frame* m_pFrame;
    
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
    
    VkPushConstantRange m_pushConstantRange;
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
};

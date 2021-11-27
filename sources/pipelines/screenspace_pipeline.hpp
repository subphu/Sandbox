//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/renderpass.hpp"
#include "../resources/frame.hpp"
#include "../window/gui.hpp"
#include "../window/window.hpp"

class ScreenSpacePipeline {
    
public:
    ~ScreenSpacePipeline();
    ScreenSpacePipeline();
    
    void cleanup();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    void createGUI(Window* pWindow);
    
    void setFrame(Frame* pFrame);
    
    Renderpass* getRenderpass();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Renderpass* m_pRenderpass;
    Frame* m_pFrame;
    GUI* m_pGUI;
    
    VkGraphicsPipelineCreateInfo m_pipelineInfo{};

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
    
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
};

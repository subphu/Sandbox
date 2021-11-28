//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/renderpass.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/frame.hpp"
#include "../resources/image.hpp"
#include "../window/gui.hpp"
#include "../window/window.hpp"

class ScreenSpacePipeline {
    
public:
    ~ScreenSpacePipeline();
    ScreenSpacePipeline();
    
    void cleanup();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput(Image* pImage);
    
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    void createGUI(Window* pWindow);
    
    void setFrame(Frame* pFrame);
    
    Renderpass* getRenderpass();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Pipeline* m_pPipeline;
    Renderpass* m_pRenderpass;
    Descriptor* m_pDescriptor;
    
    Image* m_pRenderImage;
    Frame* m_pFrame;
    GUI* m_pGUI;
    
    VkViewport m_viewport{};
    VkRect2D   m_scissor{};
    
    VkPipelineLayout m_pipelineLayout;
    
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
  
    void updateViewportScissor();
    
};

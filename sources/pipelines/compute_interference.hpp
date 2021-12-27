//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class ComputeInterference {

    struct PCMisc {
        uint opdSample;
        uint rSample;
    };

public:
    ~ComputeInterference();
    ComputeInterference();
    
    void cleanup();
    void dispatch();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    void setupOutput();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
    Image* copyOutputImage();
    
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Image*  m_pOutputImage;
    
    PCMisc m_misc;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class ComputeHDR {

    struct PCMisc {
        UInt2D size;
    };

public:
    ~ComputeHDR();
    ComputeHDR();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    Image* dispatch();
    
    void setupShader();
    void setupInputOutput(std::string hdrPath);
    void cleanInputOutput();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Image*  m_pOutputImage;
    Buffer* m_pInputBuffer;
    
    PCMisc m_misc;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
    
};

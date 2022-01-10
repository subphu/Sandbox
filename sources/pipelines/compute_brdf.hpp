//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class ComputeBRDF {

    struct PCMisc {
        UInt2D size;
    };

public:
    ~ComputeBRDF();
    ComputeBRDF();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    Image* dispatch(UInt2D size);
    
    void setupShader();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    PCMisc m_misc;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
    
};

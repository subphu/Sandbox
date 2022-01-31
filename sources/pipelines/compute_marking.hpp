//  Copyright © 2022 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class ComputeMarking {

    struct PCMisc {
        UInt2D size;
    };

public:
    ~ComputeMarking();
    ComputeMarking();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput(Image* image, Buffer* buffer);
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
    Image* getOutputImage();
    
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Image*  m_pInputImage;
    Image*  m_pOutputImage;
    Buffer* m_pMarkBuffer;
    
    PCMisc m_misc;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
    
};

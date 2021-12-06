//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class InterferencePipeline {

    struct InterferenceDetails {
        uint sampleSize;
        float n; // refractive index
    };

public:
    ~InterferencePipeline();
    InterferencePipeline();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    void setupOutput();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
    Image*  getOutputImage();
    Buffer* getOutputBuffer();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Image*  m_pOutputImage;
    Buffer* m_pOutputBuffer;
    
    InterferenceDetails m_details;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

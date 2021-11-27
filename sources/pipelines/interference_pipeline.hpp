//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/buffer.hpp"

#define CHANNEL 4

class InterferencePipeline {

    struct InterferenceDetails {
        uint length;
        float n; // refractive index
    };

public:
    ~InterferencePipeline();
    InterferencePipeline();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput(uint length, float n);
    void setupOutput();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
    Buffer* getOutputBuffer();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Descriptor* m_pDescriptor;
    
    Buffer* m_pOutputBuffer;
    
    InterferenceDetails m_details;
    
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

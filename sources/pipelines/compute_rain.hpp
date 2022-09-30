//  Copyright © 2022 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/buffer.hpp"

class ComputeRain {

    struct PCMisc {
        glm::vec3 area;
        int amount;
        float speed;
    };

public:
    ~ComputeRain();
    ComputeRain();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    void setupOutput();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
        
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Buffer* m_pPositionBuffer;
    
    PCMisc m_misc;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

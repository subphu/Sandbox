//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class FluidPipeline {

    struct SimulationDetails {
        UInt2D size;
        uint opdSample;
    };

public:
    ~FluidPipeline();
    FluidPipeline();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    void setupOutput();
    
    void updateInterferenceInput(Buffer* pInterferenceBuffer);
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    
    Image* getFluidImage();
    Image* getHeightImage();
    Image* getIridescentImage();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Image* m_pSampledImage;
    Image* m_pFluidImage;
    Image* m_pHeightImage;
    Image* m_pIridescentImage;
    
    Buffer* m_pInterferenceBuffer;
    
    SimulationDetails m_details;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

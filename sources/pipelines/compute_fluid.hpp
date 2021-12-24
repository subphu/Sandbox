//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/image.hpp"
#include "../resources/buffer.hpp"

class ComputeFluid {

    struct PCMisc {
        UInt2D size;
        float reflectance;
    };

public:
    ~ComputeFluid();
    ComputeFluid();
    
    void cleanup();
    void dispatch(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    void setupOutput();
    
    void updateInterferenceInput(Image* pInterferenceImage);
    
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
    
    Image* m_pInterferenceImage;
    
    PCMisc m_details;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
};

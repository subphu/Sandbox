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
    
    std::string getTextureName();
    std::string getHDRTexturePath();
    std::string getEnvTexturePath();
    
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Descriptor* m_pDescriptor;
    
    Image*  m_pOutputImage;
    Buffer* m_pInputBuffer;
    
    PCMisc m_misc;
    
    uint m_textureIdx = 0;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VkPipelineShaderStageCreateInfo m_shaderStage;
    
private:
    const std::vector<std::string> TEXTURE_NAMES = {"Arches_E_PineTree", "GravelPlaza",  "Tokyo_BigSight", "Ueno-Shrine"};
    const std::string TEXTURE_HDR_PATH = ".hdr";
    const std::string TEXTURE_ENV_PATH = "_Env.hdr";
};

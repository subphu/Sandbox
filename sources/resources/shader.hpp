//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "device.hpp"

class Shader {
    
public:
    ~Shader();
    Shader();
    Shader(const std::string filepath, VkShaderStageFlagBits stage, const char* entryPoint = "main");
    
    void cleanup();
    
    std::string m_filepath;
    
    void createModule(const std::string filepath);
    void createStageInfo(VkShaderStageFlagBits stage, const char* entryPoint = "main");
    
    VkPipelineShaderStageCreateInfo getShaderStageInfo();
    
    VkShaderModuleCreateInfo        m_shaderInfo{};
    VkPipelineShaderStageCreateInfo m_shaderStageInfo{};
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    
    
    std::vector<char> ReadBinaryFile(const std::string filename);
    
};

//  Copyright © 2021 Subph. All rights reserved.
//

#include "shader.hpp"

#include "../system.hpp"

#include <fstream>

Shader::~Shader() {}
Shader::Shader() : m_pDevice(System::Device()) {}

Shader::Shader(const std::string filepath, VkShaderStageFlagBits stage, const char* entryPoint) : m_pDevice(System::Device())  {
    createModule(filepath);
    createStageInfo(stage, entryPoint);
}

void Shader::cleanup() {
    LOG("Shader::cleanup");
    m_cleaner.flush();
}

void Shader::createModule(const std::string filepath) {
    LOG("Shader::createModule");
    VkDevice device = m_pDevice->getDevice();
    auto code = ReadBinaryFile(filepath);
    
    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = code.size();
    shaderInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule);
    CHECK_VKRESULT(result, "failed to create shader modul!");
    m_cleaner.push([=](){ vkDestroyShaderModule(device, shaderModule, nullptr); });
    
    m_filepath     = filepath;
    m_shaderModule = shaderModule;
}

void Shader::createStageInfo(VkShaderStageFlagBits stage, const char* entryPoint) {
    VkShaderModule shaderModule = m_shaderModule;
    
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage  = stage;
    shaderStageInfo.pName  = entryPoint;
    shaderStageInfo.module = shaderModule;
    
    m_shaderStageInfo = shaderStageInfo;
}

VkPipelineShaderStageCreateInfo Shader::getShaderStageInfo() {
    return m_shaderStageInfo;
}

VECTOR<char> Shader::ReadBinaryFile(const std::string filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("failed to open file!");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

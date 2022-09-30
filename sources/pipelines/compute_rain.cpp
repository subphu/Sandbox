//  Copyright © 2022 Subph. All rights reserved.
//

#include "compute_rain.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

#define WORKGROUP_SIZE_X 16
#define WORKGROUP_SIZE_Y 16

ComputeRain::~ComputeRain() {}
ComputeRain::ComputeRain() {}

void ComputeRain::cleanup() { m_cleaner.flush("ComputeRain"); }

void ComputeRain::setupShader() {
    LOG("ComputeRain::setupShader");
    Shader* compShader = new Shader(SPIRV_PATH + "rain.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    m_shaderStage = compShader->getShaderStageInfo();
    m_cleaner.push([=](){ compShader->cleanup(); });
}

void ComputeRain::setupInput() {
    LOG("ComputeRain::setupInput");
    
    m_misc.area = {2.f, 5.f, 2.f};
    m_misc.amount = 100;
    m_misc.speed = 0.001f;
    
    VECTOR<glm::vec4> positions;
    for (int i = 0; i < m_misc.amount; i++) {
        positions.push_back({
            (FLOAT(rand()) / FLOAT(RAND_MAX) - 0.5f) * m_misc.area.x,
            (FLOAT(rand()) / FLOAT(RAND_MAX) - 0.5f) * m_misc.area.y,
            (FLOAT(rand()) / FLOAT(RAND_MAX) - 0.5f) * m_misc.area.z,
            0.f
        });
    }
    
    uint bufferSize = m_misc.amount * sizeof(glm::vec4);
    m_pPositionBuffer = new Buffer();
    m_pPositionBuffer->setup(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    m_pPositionBuffer->create();
    m_pPositionBuffer->fillBufferFull(positions.data());
    m_cleaner.push([=](){ m_pPositionBuffer->cleanup(); });
    
    m_pDescriptor->setupPointerBuffer(S0, B0, m_pPositionBuffer->getDescriptorInfo());
    m_pDescriptor->update(S0);
}

void ComputeRain::createDescriptor() {
    LOG("ComputeRain::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->createLayout(S0);
    
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void ComputeRain::createPipelineLayout() {
    LOG("ComputeRain::createPipelineLayout");
    VkDevice device = System::Device()->getDevice();
    VkDescriptorSetLayout descSetLayout = m_pDescriptor->getDescriptorLayout(S0);
    
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(PCMisc);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
    
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    m_cleaner.push([=](){ vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr); });
}

void ComputeRain::createPipeline() {
    LOG("ComputeRain::createPipeline");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineShaderStageCreateInfo shaderStage = m_shaderStage;
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages({shaderStage});
    m_pPipeline->createComputePipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void ComputeRain::dispatch(VkCommandBuffer cmdBuffer) {
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipeline       pipeline = m_pPipeline->get();
    VkDescriptorSet  descSet  = m_pDescriptor->getDescriptorSet(S0);
    PCMisc           misc     = m_misc;
    
    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(PCMisc), &misc);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descSet, 0, nullptr);
    
    vkCmdDispatch(cmdBuffer,
                  misc.amount  / WORKGROUP_SIZE_X + 1,
                  1, 1);
    
}

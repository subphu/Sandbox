//  Copyright © 2022 Subph. All rights reserved.
//

#include "compute_brdf.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

#define WORKGROUP_SIZE_X 16
#define WORKGROUP_SIZE_Y 16

ComputeBRDF::~ComputeBRDF() {}
ComputeBRDF::ComputeBRDF() {}

void ComputeBRDF::cleanup() { m_cleaner.flush("ComputeBRDF"); }

void ComputeBRDF::setupShader() {
    LOG("ComputeBRDF::setupShader");
    Shader* compShader = new Shader(SPIRV_PATH + "brdf.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    m_shaderStage = compShader->getShaderStageInfo();
    m_cleaner.push([=](){ compShader->cleanup(); });
}

void ComputeBRDF::createDescriptor() {
    LOG("ComputeBRDF::createDescriptor");
    m_pDescriptor = new Descriptor();
    
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->createLayout(S0);
    m_pDescriptor->createPool();

    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void ComputeBRDF::createPipelineLayout() {
    LOG("ComputeBRDF::createPipelineLayout");
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

void ComputeBRDF::createPipeline() {
    LOG("ComputeBRDF::createPipeline");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineShaderStageCreateInfo shaderStage = m_shaderStage;
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages({shaderStage});
    m_pPipeline->createComputePipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

Image* ComputeBRDF::dispatch(UInt2D size) {
    m_misc.size = size;
    Image* imageOutput = new Image();
    imageOutput->setupForHDRTexture(size);
    imageOutput->createWithSampler();
    imageOutput->cmdTransitionToStorageW();
    
    m_pDescriptor->setupPointerImage(S0, B0, imageOutput->getDescriptorInfo());
    m_pDescriptor->update(S0);
    
    Commander* pCommander = System::Commander();
    VkCommandBuffer cmdBuffer = pCommander->createCommandBuffer();
    pCommander->beginSingleTimeCommands(cmdBuffer);
    dispatch(cmdBuffer);
    imageOutput->cmdTransitionToShaderR(cmdBuffer);
    pCommander->endSingleTimeCommands(cmdBuffer);
    
    return imageOutput;
}

void ComputeBRDF::dispatch(VkCommandBuffer cmdBuffer) {
    LOG("ComputeBRDF::dispatch");
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
                  misc.size.width  / WORKGROUP_SIZE_X + 1,
                  misc.size.height / WORKGROUP_SIZE_Y + 1, 1);
    
}

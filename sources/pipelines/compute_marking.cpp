//  Copyright © 2022 Subph. All rights reserved.
//

#include "compute_marking.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

#define WORKGROUP_SIZE_X 16
#define WORKGROUP_SIZE_Y 16

ComputeMarking::~ComputeMarking() {}
ComputeMarking::ComputeMarking() {}

void ComputeMarking::cleanup() { m_cleaner.flush("ComputeMarking"); }

void ComputeMarking::setupShader() {
    LOG("ComputeMarking::setupShader");
    Shader* compShader = new Shader(SPIRV_PATH + "marking.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    m_shaderStage = compShader->getShaderStageInfo();
    m_cleaner.push([=](){ compShader->cleanup(); });
}

void ComputeMarking::setupInput(Image* image, Buffer *buffer) {
    LOG("ComputeMarking::setupInput");
    m_misc.size = image->getImageSize();
    m_pMarkBuffer = buffer;
    m_pInputImage = image;
    
    m_pOutputImage = new Image();
    m_pOutputImage->setupForStorage({m_misc.size.width, 1});
    m_pOutputImage->createWithSampler();
    m_pOutputImage->cmdTransitionToStorageW();
    m_cleaner.push([=](){ m_pOutputImage->cleanup(); });
    
    m_pDescriptor->setupPointerBuffer(S0, B0, m_pMarkBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B1, m_pInputImage->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B2, m_pOutputImage->getDescriptorInfo());
    m_pDescriptor->update(S0);
}

void ComputeMarking::createDescriptor() {
    LOG("ComputeMarking::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->createLayout(S0);
    
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void ComputeMarking::createPipelineLayout() {
    LOG("ComputeMarking::createPipelineLayout");
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

void ComputeMarking::createPipeline() {
    LOG("ComputeMarking::createPipeline");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineShaderStageCreateInfo shaderStage = m_shaderStage;
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages({shaderStage});
    m_pPipeline->createComputePipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void ComputeMarking::dispatch(VkCommandBuffer cmdBuffer) {
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
                  1, 1);
    
}

Image* ComputeMarking::getOutputImage() { return m_pOutputImage; }

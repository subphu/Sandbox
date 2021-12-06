//  Copyright © 2021 Subph. All rights reserved.
//

#include "interference_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

#define WORKGROUP_SIZE_X 128

InterferencePipeline::~InterferencePipeline() {}
InterferencePipeline::InterferencePipeline() : m_pDevice(System::Device()) {}

void InterferencePipeline::cleanup() { m_cleaner.flush("InterferencePipeline"); }

void InterferencePipeline::setupShader() {
    LOG("InterferencePipeline::setupShader");
    Shader* compShader = new Shader(SPIRV_PATH + "interference1d.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    m_shaderStage = compShader->getShaderStageInfo();
    m_cleaner.push([=](){ compShader->cleanup(); });
}

void InterferencePipeline::setupInput(uint sampleSize, float n) {
    m_details.n = n;
    m_details.sampleSize = sampleSize;
}

void InterferencePipeline::setupOutput() {
    LOG("InterferencePipeline::setupOutput");
    uint floatCount = m_details.sampleSize * CHANNEL;
    uint outputSize = floatCount * sizeof(float);
    std::vector<float> outputData(floatCount, 0.0f);
    
    m_pOutputImage = new Image();
    m_pOutputImage->setupForStorage({m_details.sampleSize / WORKGROUP_SIZE_X , 1});
    m_pOutputImage->createWithSampler();
    m_pOutputImage->cmdTransitionToStorageW();
    
    m_pOutputBuffer = new Buffer();
    m_pOutputBuffer->setup(outputSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    m_pOutputBuffer->create();
    m_pOutputBuffer->fillBufferFull(outputData.data());
    
    m_pDescriptor->setupPointerBuffer(S0, B0, m_pOutputBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B1, m_pOutputImage->getDescriptorInfo());
    m_pDescriptor->update(S0);
}

void InterferencePipeline::createDescriptor() {
    LOG("InterferencePipeline::createDescriptor");
    m_pDescriptor = new Descriptor();
    
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->createLayout(S0);
    m_pDescriptor->createPool();

    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void InterferencePipeline::createPipelineLayout() {
    LOG("InterferencePipeline::createPipelineLayout");
    VkDevice device = m_pDevice->getDevice();
    VkDescriptorSetLayout descSetLayout = m_pDescriptor->getDescriptorLayout(S0);
    
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(InterferenceDetails);
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

void InterferencePipeline::createPipeline() {
    LOG("InterferencePipeline::createPipeline");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineShaderStageCreateInfo shaderStage = m_shaderStage;
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages({shaderStage});
    m_pPipeline->createComputePipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void InterferencePipeline::dispatch(VkCommandBuffer cmdBuffer) {
    LOG("InterferencePipeline::dispatch");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipeline          pipeline = m_pPipeline->get();
    VkDescriptorSet     descSet  = m_pDescriptor->getDescriptorSet(S0);
    InterferenceDetails details  = m_details;
    
    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(InterferenceDetails), &details);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descSet, 0, nullptr);
    
    vkCmdDispatch(cmdBuffer, details.sampleSize / WORKGROUP_SIZE_X, 1, 1);
}

Image * InterferencePipeline::getOutputImage () { return m_pOutputImage; }
Buffer* InterferencePipeline::getOutputBuffer() { return m_pOutputBuffer; }

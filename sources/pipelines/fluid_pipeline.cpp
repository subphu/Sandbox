//  Copyright © 2021 Subph. All rights reserved.
//

#include "fluid_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

#define WORKGROUP_SIZE_X 16
#define WORKGROUP_SIZE_Y 16

FluidPipeline::~FluidPipeline() {}
FluidPipeline::FluidPipeline() : m_pDevice(System::Device()) {}

void FluidPipeline::cleanup() { m_cleaner.flush("FluidPipeline"); }

void FluidPipeline::setupShader() {
    LOG("FluidPipeline::setupShader");
    Shader* compShader = new Shader(SPIRV_PATH + "fluid.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    m_shaderStage = compShader->getShaderStageInfo();
    m_cleaner.push([=](){ compShader->cleanup(); });
}

void FluidPipeline::setupInput() {
    m_details.opdSample = System::Settings()->OPDSample;
    m_details.size = System::Settings()->FluidSize;
    
    uint outputSize = m_details.opdSample * CHANNEL * sizeof(float);
    m_pInterferenceBuffer = new Buffer();
    m_pInterferenceBuffer->setup(outputSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_pInterferenceBuffer->create();
    m_pDescriptor->setupPointerBuffer(S0, B9, m_pInterferenceBuffer->getDescriptorInfo());
    m_cleaner.push([=](){ m_pInterferenceBuffer->cleanup(); });
}

void FluidPipeline::updateInterferenceInput(Buffer* pInterferenceBuffer) {
    m_pInterferenceBuffer->cmdCopyFromBuffer(pInterferenceBuffer->get(), pInterferenceBuffer->getBufferSize());
}

void FluidPipeline::setupOutput() {
    m_pSampledImage = new Image();
    m_pSampledImage->setupForStorage(m_details.size);
    m_pSampledImage->createWithSampler();
    m_pSampledImage->cmdClearColorImage();
    m_pSampledImage->cmdTransitionToShaderR();
    
    m_pFluidImage = new Image();
    m_pFluidImage->setupForStorage(m_details.size);
    m_pFluidImage->createWithSampler();
    m_pFluidImage->cmdClearColorImage();
    m_pFluidImage->cmdTransitionToStorageW();
    
    m_pHeightImage = new Image();
    m_pHeightImage->setupForStorage(m_details.size);
    m_pHeightImage->createWithSampler();
    m_pHeightImage->cmdClearColorImage();
    m_pHeightImage->cmdTransitionToStorageW();
    
    m_pIridescentImage = new Image();
    m_pIridescentImage->setupForStorage(m_details.size);
    m_pIridescentImage->createWithSampler();
    m_pIridescentImage->cmdClearColorImage();
    m_pIridescentImage->cmdTransitionToStorageW();
    
    m_pDescriptor->setupPointerImage(S0, B0, m_pSampledImage->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B1, m_pFluidImage->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B2, m_pHeightImage->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B3, m_pIridescentImage->getDescriptorInfo());
    m_pDescriptor->update(S0);
    
    m_cleaner.push([=](){ m_pSampledImage->cleanup(); });
    m_cleaner.push([=](){ m_pFluidImage->cleanup(); });
    m_cleaner.push([=](){ m_pHeightImage->cleanup(); });
    m_cleaner.push([=](){ m_pIridescentImage->cleanup(); });
}

void FluidPipeline::createDescriptor() {
    LOG("FluidPipeline::createDescriptor");
    m_pDescriptor = new Descriptor();
    
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->addLayoutBindings(S0, B9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT);
    m_pDescriptor->createLayout(S0);
    m_pDescriptor->createPool();

    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void FluidPipeline::createPipelineLayout() {
    LOG("FluidPipeline::createPipelineLayout");
    VkDevice device = m_pDevice->getDevice();
    VkDescriptorSetLayout descSetLayout = m_pDescriptor->getDescriptorLayout(S0);
    
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(SimulationDetails);
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

void FluidPipeline::createPipeline() {
    LOG("FluidPipeline::createPipeline");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineShaderStageCreateInfo shaderStage = m_shaderStage;
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages({shaderStage});
    m_pPipeline->createComputePipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void FluidPipeline::dispatch(VkCommandBuffer cmdBuffer) {
    VkPipelineLayout  pipelineLayout = m_pipelineLayout;
    VkPipeline        pipeline = m_pPipeline->get();
    VkDescriptorSet   descSet  = m_pDescriptor->getDescriptorSet(S0);
    SimulationDetails details  = m_details;
    
    m_pFluidImage->cmdTransitionToStorageW(cmdBuffer);
    m_pHeightImage->cmdTransitionToStorageW(cmdBuffer);
    m_pIridescentImage->cmdTransitionToStorageW(cmdBuffer);
    
    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(SimulationDetails), &details);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descSet, 0, nullptr);
    
    vkCmdDispatch(cmdBuffer,
                  details.size.width  / WORKGROUP_SIZE_X,
                  details.size.height / WORKGROUP_SIZE_Y, 1);
    
    m_pFluidImage->cmdTransitionToTransferSrc(cmdBuffer);
    m_pSampledImage->cmdTransitionToTransferDst(cmdBuffer);
    
    m_pSampledImage->cmdCopyImageToImage(cmdBuffer, m_pFluidImage);
    
    m_pSampledImage->cmdTransitionToShaderR(cmdBuffer);
    m_pFluidImage->cmdTransitionToShaderR(cmdBuffer);
    m_pHeightImage->cmdTransitionToShaderR(cmdBuffer);
    m_pIridescentImage->cmdTransitionToShaderR(cmdBuffer);
}

Image * FluidPipeline::getFluidImage     () { return m_pFluidImage;  }
Image * FluidPipeline::getHeightImage    () { return m_pHeightImage; }
Image * FluidPipeline::getIridescentImage() { return m_pIridescentImage; }

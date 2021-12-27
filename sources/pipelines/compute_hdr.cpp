//  Copyright © 2021 Subph. All rights reserved.
//

#include "compute_hdr.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

#define WORKGROUP_SIZE_X 16
#define WORKGROUP_SIZE_Y 16

ComputeHDR::~ComputeHDR() {}
ComputeHDR::ComputeHDR() {}

void ComputeHDR::cleanup() { m_cleaner.flush("ComputeHDR"); }

void ComputeHDR::setupShader() {
    LOG("ComputeHDR::setupShader");
    Shader* compShader = new Shader(SPIRV_PATH + "hdr.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    m_shaderStage = compShader->getShaderStageInfo();
    m_cleaner.push([=](){ compShader->cleanup(); });
}

void ComputeHDR::setupInputOutput() {
    LOG("ComputeHDR::setupInputOutput");
    std::string hdrPath = getHDRTexturePath();
    
    m_pOutputImage = new Image();
    m_pOutputImage->setupForHDRTexture(hdrPath);
    m_pOutputImage->createWithSampler();
    m_pOutputImage->cmdTransitionToStorageW();
    m_cleaner.push([=](){ m_pOutputImage->cleanup(); });
    
    float* imageData = m_pOutputImage->getRawHDR();
    UInt2D imageSize = m_pOutputImage->getImageSize();
    uint channelSize = m_pOutputImage->getRawChannel() * sizeof(float);
    VkDeviceSize deviceSize = imageSize.width * imageSize.height * channelSize;
    
    m_pInputBuffer = new Buffer();
    m_pInputBuffer->setup(deviceSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    m_pInputBuffer->create();
    m_pInputBuffer->fillBufferFull(imageData);
    m_cleaner.push([=](){ m_pInputBuffer->cleanup(); });
    
    m_pDescriptor->setupPointerBuffer(S0, B0, m_pInputBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S0, B1, m_pOutputImage->getDescriptorInfo());
    m_pDescriptor->update(S0);
    
    m_misc.size = imageSize;
}

void ComputeHDR::createDescriptor() {
    LOG("ComputeHDR::createDescriptor");
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

void ComputeHDR::createPipelineLayout() {
    LOG("ComputeHDR::createPipelineLayout");
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

void ComputeHDR::createPipeline() {
    LOG("ComputeHDR::createPipeline");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineShaderStageCreateInfo shaderStage = m_shaderStage;
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages({shaderStage});
    m_pPipeline->createComputePipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void ComputeHDR::dispatch(VkCommandBuffer cmdBuffer) {
    LOG("ComputeHDR::dispatch");
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipeline       pipeline = m_pPipeline->get();
    VkDescriptorSet  descSet  = m_pDescriptor->getDescriptorSet(S0);
    PCMisc           misc     = m_misc;
    
    m_pOutputImage->cmdTransitionToStorageW();
    
    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(PCMisc), &misc);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descSet, 0, nullptr);
    
    vkCmdDispatch(cmdBuffer,
                  misc.size.width  / WORKGROUP_SIZE_X + 1,
                  misc.size.height / WORKGROUP_SIZE_Y + 1, 1);
    
    m_pOutputImage->cmdTransitionToTransferSrc(cmdBuffer);
}

Image* ComputeHDR::copyOutputImage () {
    UInt2D imageSize = m_pOutputImage->getImageSize();
    Image* imageCopy = new Image();
    imageCopy->setupForHDRTexture(imageSize);
    imageCopy->createWithSampler();
    
    Commander* pCommander = System::Commander();
    VkCommandBuffer cmdBuffer = pCommander->createCommandBuffer();
    pCommander->beginSingleTimeCommands(cmdBuffer);
    imageCopy->cmdTransitionToTransferDst(cmdBuffer);
    imageCopy->cmdCopyImageToImage(cmdBuffer, m_pOutputImage);
    imageCopy->cmdGenerateMipmaps(cmdBuffer);
    pCommander->endSingleTimeCommands(cmdBuffer);
    
    return imageCopy;
}

std::string ComputeHDR::getTextureName() { return TEXTURE_NAMES[textureIdx] + "/" + TEXTURE_NAMES[textureIdx]; }
std::string ComputeHDR::getHDRTexturePath() { return CUBE_PATH + getTextureName() + TEXTURE_HDR_PATH; }
std::string ComputeHDR::getEnvTexturePath() { return CUBE_PATH + getTextureName() + TEXTURE_ENV_PATH; }

//  Copyright © 2021 Subph. All rights reserved.
//

#include "graphics_equirect.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"
#include "../resources/camera.hpp"

GraphicsEquirect::~GraphicsEquirect() {}
GraphicsEquirect::GraphicsEquirect() {}

void GraphicsEquirect::cleanup() { m_cleaner.flush("GraphicsEquirect"); }

void GraphicsEquirect::render(VkCommandBuffer cmdBuffer) {
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipeline       pipeline       = m_pPipeline->get();
    VkRenderPass     renderpass     = m_pRenderpass->get();
    VkFramebuffer    framebuffer    = m_pFrame->getFramebuffer();
    VkRect2D         scissor        = m_scissor;
    VkViewport       viewport       = m_viewport;
    
    VkDeviceSize offsets  = 0;
    VkBuffer vertexBuffer = m_pCube->getVertexBuffer()->get();
    VkBuffer indexBuffer  = m_pCube->getIndexBuffer()->get();
    uint32_t indexSize    = m_pCube->getIndexSize();
    
    VkDescriptorSet hdrDescSet  = m_pDescriptor->getDescriptorSet(S0);
    
    VkClearValue clearValue{ VEC4_BLACK };
    
    VkRenderPassBeginInfo renderBeginInfo{};
    renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo.clearValueCount = 1;
    renderBeginInfo.pClearValues    = &clearValue;
    renderBeginInfo.renderPass      = renderpass;
    renderBeginInfo.framebuffer     = framebuffer;
    renderBeginInfo.renderArea      = scissor;
    
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
    vkCmdBeginRenderPass(cmdBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S0, 1, &hdrDescSet, 0, nullptr);
    
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer, &offsets);
    vkCmdBindIndexBuffer  (cmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    for (int i = 0; i < 6; i++) {
        m_misc.layer = i;
        m_misc.mvp = CUBEMAP_PROJ * CUBEMAP_VIEWS[i];
        vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PCMisc), &m_misc);
        vkCmdDrawIndexed(cmdBuffer, indexSize, 1, 0, 0, 0);
    }
    
    vkCmdEndRenderPass(cmdBuffer);
}

void GraphicsEquirect::render() {
    Commander* pCommander = System::Commander();
    VkCommandBuffer cmdBuffer = pCommander->createCommandBuffer();
    pCommander->beginSingleTimeCommands(cmdBuffer);
    render(cmdBuffer);
    pCommander->endSingleTimeCommands(cmdBuffer);
}

void GraphicsEquirect::setupShader() {
    LOG("GraphicsEquirect::setupShader");
    Shader* vertShader = new Shader(SPIRV_PATH + "equirect.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader(SPIRV_PATH + "equirect.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); });
}

void GraphicsEquirect::setupMesh() {
    m_pCube = new Mesh();
    m_pCube->createCube();
    m_pCube->createVertexBuffer();
    m_pCube->createIndexBuffer();
    m_pCube->createVertexStateInfo();
    m_cleaner.push([=](){ m_pCube->cleanup(); });
}

void GraphicsEquirect::setupInput(Image* equirect) {
    LOG("GraphicsEquirect::setupInput");
    m_pEquirect = equirect;
    m_pEquirect->cmdTransitionToShaderR();
    m_pDescriptor->setupPointerImage(S0, B0, m_pEquirect->getDescriptorInfo());
    m_pDescriptor->update(S0);
}

void GraphicsEquirect::createDescriptor() {
    LOG("GraphicsEquirect::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S0);
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void GraphicsEquirect::createRenderpass() {
    m_pRenderpass = new Renderpass();
    m_pRenderpass->setupColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);
    m_pRenderpass->setup();
    m_pRenderpass->create();
    m_cleaner.push([=](){ m_pRenderpass->cleanup(); });
}

void GraphicsEquirect::createPipelineLayout() {
    LOG("GraphicsEquirect::createPipelineLayout");
    VkDevice device = System::Device()->getDevice();
    VkDescriptorSetLayout descSetLayout = m_pDescriptor->getDescriptorLayout(S0);
    
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(PCMisc);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    
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

void GraphicsEquirect::createPipeline() {
    LOG("GraphicsEquirect::createPipeline");
    VkRenderPass renderpass = m_pRenderpass->get();
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VECTOR<VkPipelineShaderStageCreateInfo> shaderStages = m_shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = m_pCube->getVertexStateInfo();
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setRenderpass(renderpass);
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages(shaderStages);
    m_pPipeline->setVertexInputInfo(vertexInputInfo);
    
    m_pPipeline->setupViewportInfo();
    m_pPipeline->setupInputAssemblyInfo();
    m_pPipeline->setupRasterizationInfo();
    m_pPipeline->setupMultisampleInfo();
    
    m_pPipeline->setupBlendAttachment(VK_FALSE);
    m_pPipeline->setupColorBlendInfo();
    
    m_pPipeline->setupDynamicInfo();

    m_pPipeline->createGraphicsPipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void GraphicsEquirect::cleanFrame() { m_pFrame->cleanup(); }
void GraphicsEquirect::createFrame(uint32_t size) {
    LOG("GraphicsEquirect::createFrame");
    m_pFrame = new Frame({size, size});
    m_pFrame->createCubeResource();
    m_pFrame->createFramebuffer(m_pRenderpass);
    m_cleaner.push([=](){ m_pFrame->cleanup(); });
    updateViewportScissor();
}

Image* GraphicsEquirect::copyFrameImage() {
    UInt2D imageSize = m_pFrame->getSize();
    Image* imageFrame = m_pFrame->getColorImage();
    Image* imageCopy = new Image();
    imageCopy->setupForCubemap(imageSize);
    imageCopy->createWithSampler();
    
    Commander* pCommander = System::Commander();
    VkCommandBuffer cmdBuffer = pCommander->createCommandBuffer();
    pCommander->beginSingleTimeCommands(cmdBuffer);
    imageFrame->cmdTransitionToTransferSrc(cmdBuffer);
    imageCopy->cmdTransitionToTransferDst(cmdBuffer);
    imageCopy->cmdCopyImageToImage(cmdBuffer, imageFrame);
    imageCopy->cmdGenerateMipmaps(cmdBuffer);
    pCommander->endSingleTimeCommands(cmdBuffer);
    
    return imageCopy;
}

void GraphicsEquirect::updateViewportScissor() {
    UInt2D extent = m_pFrame->getSize();
    m_viewport.x = 0.f;
    m_viewport.y = 0.f;
    m_viewport.width  = extent.width;
    m_viewport.height = extent.height;
    m_viewport.minDepth = 0.f;
    m_viewport.maxDepth = 1.f;
    m_scissor.offset = {0, 0};
    m_scissor.extent = extent;
}

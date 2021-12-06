//  Copyright © 2021 Subph. All rights reserved.
//

#include "screenspace_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"


ScreenSpacePipeline::~ScreenSpacePipeline() {}
ScreenSpacePipeline::ScreenSpacePipeline() : m_pDevice(System::Device()) {}

void ScreenSpacePipeline::cleanup() { m_cleaner.flush("ScreenSpacePipeline"); }

void ScreenSpacePipeline::render(VkCommandBuffer cmdBuffer) {
    Image*           pInputImage    = m_pInputFrame->getColorImage();;
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipeline       pipeline       = m_pPipeline->get();
    VkRenderPass     renderpass     = m_pRenderpass->get();
    VkFramebuffer    framebuffer    = m_pFrame->getFramebuffer();
    VkRect2D         scissor        = m_scissor;
    VkViewport       viewport       = m_viewport;
    
    VkDescriptorSet textureDescSet = m_pDescriptor->getDescriptorSet(S0);
    
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    
    VkRenderPassBeginInfo renderBeginInfo{};
    renderBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo.clearValueCount = UINT32(clearValues.size());
    renderBeginInfo.pClearValues    = clearValues.data();
    renderBeginInfo.renderPass      = renderpass;
    renderBeginInfo.framebuffer     = framebuffer;
    renderBeginInfo.renderArea      = scissor;
    
    pInputImage->cmdTransitionToShaderR(cmdBuffer);
    
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
    vkCmdBeginRenderPass(cmdBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S0, 1, &textureDescSet, 0, nullptr);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
    
    m_pGUI->renderGUI(cmdBuffer);
    
    vkCmdEndRenderPass(cmdBuffer);
    
    pInputImage->cmdTransitionToPresent(cmdBuffer);
}

void ScreenSpacePipeline::setupShader() {
    LOG("ScreenSpacePipeline::setupShader");
    Shader* vertShader = new Shader(SPIRV_PATH + "swapchain.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader(SPIRV_PATH + "swapchain.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); });
}

void ScreenSpacePipeline::setupInput(Frame* pFrame) {
    LOG("ScreenSpacePipeline::setupInput");
    Image* pImage = pFrame->getColorImage();
    pImage->cmdTransitionToShaderR();
    m_pDescriptor->setupPointerImage(S0, B0, pImage->getDescriptorInfo());
    m_pDescriptor->update(S0);
    pImage->cmdTransitionToPresent();
    m_pInputFrame = pFrame;
}

void ScreenSpacePipeline::setupGUIInput(Image *pImage) {
    m_pGUI->addInterferenceImage(pImage);
}

void ScreenSpacePipeline::createDescriptor() {
    LOG("ScreenSpacePipeline::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S0);
    
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(S0);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void ScreenSpacePipeline::createRenderpass() {
    LOG("ScreenSpacePipeline::createRenderpass");
    VkSurfaceFormatKHR surfaceFormat = m_pDevice->getSurfaceFormat();
    m_pRenderpass = new Renderpass();
    m_pRenderpass->setupColorAttachment(surfaceFormat.format);
    m_pRenderpass->setup();
    m_pRenderpass->create();
    m_cleaner.push([=](){ m_pRenderpass->cleanup(); });
}

void ScreenSpacePipeline::createPipelineLayout() {
    LOG("ScreenSpacePipeline::createPipelineLayout");
    VkDevice device = m_pDevice->getDevice();
    VkDescriptorSetLayout descSetLayout = m_pDescriptor->getDescriptorLayout(S0);
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descSetLayout;
    
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    m_cleaner.push([=](){ vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr); });
}

void ScreenSpacePipeline::createPipeline() {
    LOG("ScreenSpacePipeline::createPipeline");
    VkRenderPass renderpass = m_pRenderpass->get();
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VECTOR<VkPipelineShaderStageCreateInfo> shaderStages = m_shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setRenderpass(renderpass);
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages(shaderStages);
    m_pPipeline->setVertexInputInfo(vertexInputInfo);
    
    m_pPipeline->setupViewportInfo();
    m_pPipeline->setupInputAssemblyInfo();
    m_pPipeline->setupRasterizationInfo();
    m_pPipeline->setupMultisampleInfo();
    
    m_pPipeline->disableBlendAttachment();
    m_pPipeline->setupColorBlendInfo();
    
    m_pPipeline->setupDynamicInfo();

    m_pPipeline->createGraphicsPipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void ScreenSpacePipeline::createGUI(Window* pWindow) {
    LOG("ScreenSpacePipeline::createGUI");
    Renderpass* pRenderpass = m_pRenderpass;
    m_pGUI = new GUI();
    m_pGUI->initGUI(pWindow, pRenderpass);
    m_cleaner.push([=](){ m_pGUI->cleanupGUI(); });
}

void ScreenSpacePipeline::updateViewportScissor() {
    UInt2D extent = m_pFrame->getExtent2D();
    m_viewport.x = 0.f;
    m_viewport.y = 0.f;
    m_viewport.width  = extent.width;
    m_viewport.height = extent.height;
    m_viewport.minDepth = 0.f;
    m_viewport.maxDepth = 1.f;
    m_scissor.offset = {0, 0};
    m_scissor.extent = extent;
}

void ScreenSpacePipeline::setFrame(Frame *pFrame) { m_pFrame = pFrame;  updateViewportScissor(); }

Renderpass* ScreenSpacePipeline::getRenderpass() { return m_pRenderpass; }



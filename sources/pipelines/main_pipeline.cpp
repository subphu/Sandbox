//  Copyright © 2021 Subph. All rights reserved.
//

#include "main_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

MainPipeline::~MainPipeline() {}
MainPipeline::MainPipeline() : m_pDevice(System::Device()) {}

void MainPipeline::cleanup() { m_cleaner.flush("InterferencePipeline"); }

void MainPipeline::render(VkCommandBuffer cmdBuffer) {
    updateViewportScissor();
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipeline       pipeline       = m_pPipeline->get();
    VkRenderPass     renderpass     = m_pRenderpass->get();
    VkFramebuffer    framebuffer    = m_pFrame->getFramebuffer();
    VkRect2D         scissor        = m_scissor;
    VkViewport       viewport       = m_viewport;
    
    VkDeviceSize offsets  = 0;
    VkBuffer vertexBuffer = m_pSphere->getVertexBuffer()->get();
    VkBuffer indexBuffer  = m_pSphere->getIndexBuffer()->get();
    uint32_t indexSize    = m_pSphere->getIndexSize();
    
    VkDescriptorSet cameraDescSet  = m_pDescriptor->getDescriptorSet(S0);
    VkDescriptorSet miscDescSet    = m_pDescriptor->getDescriptorSet(S1);
    VkDescriptorSet textureDescSet = m_pDescriptor->getDescriptorSet(S2);
    
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = System::Settings()->ClearColor;
    clearValues[1].depthStencil = System::Settings()->ClearDepth;
    
    VkRenderPassBeginInfo renderBeginInfo{};
    renderBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo.clearValueCount = UINT32(clearValues.size());
    renderBeginInfo.pClearValues    = clearValues.data();
    renderBeginInfo.renderPass      = renderpass;
    renderBeginInfo.framebuffer     = framebuffer;
    renderBeginInfo.renderArea      = scissor;
    
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
    vkCmdBeginRenderPass(cmdBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S0, 1, &cameraDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S1, 1, &miscDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S2, 1, &textureDescSet, 0, nullptr);
    
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer, &offsets);
    vkCmdBindIndexBuffer  (cmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuffer, indexSize, 1, 0, 0, 0);
    
    vkCmdEndRenderPass(cmdBuffer);
}

void MainPipeline::setupShader() {
    LOG("MainPipeline::setupShader");
    Shader* vertShader = new Shader(SPIRV_PATH + "main1d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader(SPIRV_PATH + "main1d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); });
}

void MainPipeline::setupInput(uint sampleSize) {
    LOG("MainPipeline::setupInput");
    m_misc.sampleSize = sampleSize;
    m_lights.total = 4;
    
    uint outputSize = sampleSize * CHANNEL * sizeof(float);
    m_pInterferenceBuffer = new Buffer();
    m_pInterferenceBuffer->setup(outputSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_pInterferenceBuffer->create();
    m_cleaner.push([=](){ m_pInterferenceBuffer->cleanup(); });
    
    m_pCameraBuffer = new Buffer();
    m_pCameraBuffer->setup(sizeof(CameraMatrix), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pCameraBuffer->create();
    m_cleaner.push([=](){ m_pCameraBuffer->cleanup(); });
    
    m_pMiscBuffer = new Buffer();
    m_pMiscBuffer->setup(sizeof(Misc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pMiscBuffer->create();
    m_cleaner.push([=](){ m_pMiscBuffer->cleanup(); });
    
    m_pLightBuffer = new Buffer();
    m_pLightBuffer->setup(sizeof(Lights), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pLightBuffer->create();
    m_cleaner.push([=](){ m_pLightBuffer->cleanup(); });
    
    for (std::string path : getPBRTexturePaths()) {
        Image* pTexture = new Image();
        pTexture->setupForTexture(path);
        pTexture->createForTexture();
        pTexture->cmdCopyRawDataToImage();
        m_pTextures.push_back(pTexture);
        m_cleaner.push([=](){ pTexture->cleanup(); });
    }
    
    m_pDescriptor->setupPointerBuffer(S0, B0, m_pCameraBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerBuffer(S1, B0, m_pInterferenceBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerBuffer(S1, B1, m_pMiscBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerBuffer(S1, B2, m_pLightBuffer->getDescriptorInfo());
    for (uint i = 0; i < m_pTextures.size(); i++)
        m_pDescriptor->setupPointerImage(S2, i, m_pTextures[i]->getDescriptorInfo());
    
    m_pDescriptor->update(S0);
    m_pDescriptor->update(S1);
    m_pDescriptor->update(S2);
    
    m_pSphere = new Mesh();
    m_pSphere->createSphere();
    m_pSphere->createVertexBuffer();
    m_pSphere->createIndexBuffer();
    m_pSphere->createVertexStateInfo();
    m_cleaner.push([=](){ m_pSphere->cleanup(); });
}

void MainPipeline::updateLightInput(long iteration) {
    const float distScale = 8.f;
    const float distance  = glm::radians(360.f/m_lights.total);
    m_lights.color = glm::vec4(200.0f);
    for (int i = 0; i < m_lights.total; i++) {
        m_lights.position[i].z = 8.f;
        m_lights.position[i].x = sin(iteration / 1000.f + i * distance) * distScale;
        m_lights.position[i].y = cos(iteration / 1000.f + i * distance) * distScale;
    }
    m_pLightBuffer->fillBuffer(&m_lights, sizeof(Lights));
}

void MainPipeline::updateCameraInput(Camera* pCamera) {
    UInt2D size = m_pFrame->getExtent2D();
    m_misc.viewPosition = pCamera->getPosition();
    m_cameraMatrix.view = pCamera->getViewMatrix();
    m_cameraMatrix.proj = pCamera->getProjection((float) size.width / size.height);
    m_cameraMatrix.model = m_pSphere->getMatrix();
    
    m_pMiscBuffer->fillBuffer(&m_misc, sizeof(Misc));
    m_pCameraBuffer->fillBuffer(&m_cameraMatrix, sizeof(CameraMatrix));
}

void MainPipeline::updateInterferenceInput(Buffer* pInterferenceBuffer) {
    m_pInterferenceBuffer->cmdCopyFromBuffer(pInterferenceBuffer->get(), pInterferenceBuffer->getBufferSize());
}

void MainPipeline::createDescriptor() {
    LOG("MainPipeline::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_VERTEX_BIT);
    m_pDescriptor->createLayout(S0);
    
    m_pDescriptor->setupLayout(S1);
    m_pDescriptor->addLayoutBindings(S1, B0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S1, B1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S1, B2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S1);
    
    m_pDescriptor->setupLayout(S2);
    for (uint i = 0; i < USED_TEXTURE; i++) {
        m_pDescriptor->addLayoutBindings(S2, i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                       VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    m_pDescriptor->createLayout(S2);
    
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(S0);
    m_pDescriptor->allocate(S1);
    m_pDescriptor->allocate(S2);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void MainPipeline::createRenderpass() {
    m_pRenderpass = new Renderpass();
    m_pRenderpass->setupColorAttachment();
    m_pRenderpass->setupDepthAttachment();
    m_pRenderpass->setup();
    m_pRenderpass->create();
    m_cleaner.push([=](){ m_pRenderpass->cleanup(); });
}

void MainPipeline::createPipelineLayout() {
    LOG("MainPipeline::createPipelineLayout");
    VkDevice device = m_pDevice->getDevice();
    VECTOR<VkDescriptorSetLayout> descSetLayouts = {
        m_pDescriptor->getDescriptorLayout(S0),
        m_pDescriptor->getDescriptorLayout(S1),
        m_pDescriptor->getDescriptorLayout(S2)
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = UINT32(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts    = descSetLayouts.data();
    
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    m_cleaner.push([=](){ vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr); });
}

void MainPipeline::createPipeline() {
    LOG("MainPipeline::createPipeline");
    VkRenderPass renderpass = m_pRenderpass->get();
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VECTOR<VkPipelineShaderStageCreateInfo> shaderStages = m_shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = m_pSphere->getVertexStateInfo();
    
    m_pPipeline = new Pipeline();
    m_pPipeline->setRenderpass(renderpass);
    m_pPipeline->setPipelineLayout(pipelineLayout);
    m_pPipeline->setShaderStages(shaderStages);
    m_pPipeline->setVertexInputInfo(vertexInputInfo);
    
    m_pPipeline->setupViewportInfo();
    m_pPipeline->setupInputAssemblyInfo();
    m_pPipeline->setupRasterizationInfo();
    m_pPipeline->setupMultisampleInfo();
    
    m_pPipeline->enableBlendAttachment();
    m_pPipeline->setupColorBlendInfo();
    
    m_pPipeline->setupDynamicInfo();
    m_pPipeline->setupDepthStencilInfo();

    m_pPipeline->createGraphicsPipeline();
    m_cleaner.push([=](){ m_pPipeline->cleanup(); });
}

void MainPipeline::createFrame(UInt2D size) {
    m_pFrame = new Frame(size);
    m_pFrame->createImageResource();
    m_pFrame->createDepthResource();
    m_pFrame->createFramebuffer(m_pRenderpass);
    m_cleaner.push([=](){ m_pFrame->cleanup(); });
}

void MainPipeline::updateViewportScissor() {
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

Frame* MainPipeline::getFrame() { return m_pFrame; }

std::string MainPipeline::getTextureName() { return TEXTURE_NAMES[textureIdx] + "/" + TEXTURE_NAMES[textureIdx]; }
std::string MainPipeline::getAlbedoTexturePath()    { return PBR_PATH + getTextureName() + TEXTURE_ALBEDO_PATH; }
std::string MainPipeline::getAOTexturePath()        { return PBR_PATH + getTextureName() + TEXTURE_AO_PATH; }
std::string MainPipeline::getMetallicTexturePath()  { return PBR_PATH + getTextureName() + TEXTURE_METALLIC_PATH; }
std::string MainPipeline::getNormalTexturePath()    { return PBR_PATH + getTextureName() + TEXTURE_NORMAL_PATH; }
std::string MainPipeline::getRoughnessTexturePath() { return PBR_PATH + getTextureName() + TEXTURE_ROUGHNESS_PATH; }
VECTOR<std::string> MainPipeline::getPBRTexturePaths(){ return { getAlbedoTexturePath(), getAOTexturePath(), getMetallicTexturePath(), getNormalTexturePath(), getRoughnessTexturePath() }; }

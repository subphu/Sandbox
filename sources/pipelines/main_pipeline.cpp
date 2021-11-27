//  Copyright © 2021 Subph. All rights reserved.
//

#include "main_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

MainPipeline::~MainPipeline() {}
MainPipeline::MainPipeline() : m_pDevice(System::Device()) {}

void MainPipeline::cleanup() { m_cleaner.flush("InterferencePipeline"); }

void MainPipeline::render(VkCommandBuffer cmdBuffer) {
    VkRenderPass renderpass = m_pRenderpass->get();
    VkFramebuffer framebuffer = m_pFrame->getFramebuffer();
    UInt2D extent = m_pFrame->getExtent2D();
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    
    VkRenderPassBeginInfo renderBeginInfo{};
    renderBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo.clearValueCount = UINT32(clearValues.size());
    renderBeginInfo.pClearValues    = clearValues.data();
    renderBeginInfo.renderPass      = renderpass;
    renderBeginInfo.framebuffer     = framebuffer;
    renderBeginInfo.renderArea.extent = extent;
    renderBeginInfo.renderArea.offset = {0,0};
    
    vkCmdBeginRenderPass(cmdBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    
    
    vkCmdEndRenderPass(cmdBuffer);
    
}

void MainPipeline::setupShader() {
    LOG("MainPipeline::setupShader");
    Shader* vertShader = new Shader(SPIRV_PATH + "main1d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader(SPIRV_PATH + "main1d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); });
}

void MainPipeline::setupInput(Buffer* pInterferenceBuffer) {
    LOG("MainPipeline::setupInput");
    m_pCameraBuffer = new Buffer();
    m_pCameraBuffer->setup(sizeof(Misc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pCameraBuffer->create();
    m_cleaner.push([=](){ m_pCameraBuffer->cleanup(); });
    
    m_pInterferenceBuffer = pInterferenceBuffer;
    
    m_pMiscBuffer = new Buffer();
    m_pMiscBuffer->setup(sizeof(Misc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pMiscBuffer->create();
    m_cleaner.push([=](){ m_pMiscBuffer->cleanup(); });
    
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

void MainPipeline::updateInput() {
    m_pMiscBuffer->fillBuffer(&m_misc, sizeof(Misc));
    
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
    VkDevice device = m_pDevice->getDevice();
    Renderpass* pRenderpass = m_pRenderpass;
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VECTOR<VkPipelineShaderStageCreateInfo> shaderStages = m_shaderStages;
    
    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount  = 1;

    VECTOR<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicInfo{};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = UINT32(dynamicStates.size());
    dynamicInfo.pDynamicStates    = dynamicStates.data();
    
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode    = VK_CULL_MODE_NONE;
    rasterizationInfo.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.lineWidth   = 1.0f;
    
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.attachmentCount   = 1;
    colorBlendInfo.pAttachments      = &colorBlendAttachment;
    
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable        = VK_TRUE;
    depthStencilInfo.depthWriteEnable       = VK_TRUE;
    depthStencilInfo.depthBoundsTestEnable  = VK_FALSE;
    depthStencilInfo.stencilTestEnable      = VK_TRUE;
    depthStencilInfo.depthCompareOp         = VK_COMPARE_OP_LESS;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = m_pSphere->getVertexStateInfo();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout     = pipelineLayout;
    pipelineInfo.renderPass = pRenderpass->get();
    pipelineInfo.subpass    = 0;
    pipelineInfo.stageCount = UINT32(shaderStages.size());
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pViewportState      = &viewportInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState   = &multisampleInfo;
    pipelineInfo.pColorBlendState    = &colorBlendInfo;
    pipelineInfo.pDynamicState       = &dynamicInfo;
    pipelineInfo.pDepthStencilState  = &depthStencilInfo;
    
    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline );
    CHECK_VKRESULT(result, "failed to create graphics pipeline!");
    m_cleaner.push([=](){ vkDestroyPipeline(device, pipeline, nullptr); });
    
    m_pipeline = pipeline;
}

void MainPipeline::createFrame(UInt2D size) {
    m_pFrame = new Frame(size);
    m_pFrame->createImageResource();
    m_pFrame->createDepthResource();
    m_pFrame->createFramebuffer(m_pRenderpass);
}

std::string MainPipeline::getTextureName() { return TEXTURE_NAMES[textureIdx] + "/" + TEXTURE_NAMES[textureIdx]; }
std::string MainPipeline::getAlbedoTexturePath()    { return PBR_PATH + getTextureName() + TEXTURE_ALBEDO_PATH; }
std::string MainPipeline::getAOTexturePath()        { return PBR_PATH + getTextureName() + TEXTURE_AO_PATH; }
std::string MainPipeline::getMetallicTexturePath()  { return PBR_PATH + getTextureName() + TEXTURE_METALLIC_PATH; }
std::string MainPipeline::getNormalTexturePath()    { return PBR_PATH + getTextureName() + TEXTURE_NORMAL_PATH; }
std::string MainPipeline::getRoughnessTexturePath() { return PBR_PATH + getTextureName() + TEXTURE_ROUGHNESS_PATH; }
VECTOR<std::string> MainPipeline::getPBRTexturePaths(){ return { getAlbedoTexturePath(), getAOTexturePath(), getMetallicTexturePath(), getNormalTexturePath(), getRoughnessTexturePath() }; }

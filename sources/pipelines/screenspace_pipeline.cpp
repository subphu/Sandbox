//  Copyright © 2021 Subph. All rights reserved.
//

#include "screenspace_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"


ScreenSpacePipeline::~ScreenSpacePipeline() {}
ScreenSpacePipeline::ScreenSpacePipeline() : m_pDevice(System::Device()) {}

void ScreenSpacePipeline::cleanup() { m_cleaner.flush("ScreenSpacePipeline"); }

void ScreenSpacePipeline::setupShader() {
    LOG("ScreenSpacePipeline::setupShader");
    Shader* vertShader = new Shader(SPIRV_PATH + "swapchain.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader(SPIRV_PATH + "swapchain.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); });
}

void ScreenSpacePipeline::createRenderpass() {
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
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    m_cleaner.push([=](){ vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr); });
}

void ScreenSpacePipeline::createPipeline() {
    LOG("ScreenSpacePipeline::createPipeline");
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
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.attachmentCount   = 1;
    colorBlendInfo.pAttachments      = &colorBlendAttachment;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

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
    
    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline );
    CHECK_VKRESULT(result, "failed to create graphics pipeline!");
    m_cleaner.push([=](){ vkDestroyPipeline(device, pipeline, nullptr); });
    
    m_pipeline = pipeline;
}

void ScreenSpacePipeline::render(VkCommandBuffer cmdBuffer) {
    VkRenderPass renderpass = m_pRenderpass->get();
    VkFramebuffer framebuffer = m_pFrame->getFramebuffer();
    UInt2D extent = m_pFrame->getExtent2D();
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width  = extent.width;
    viewport.height = extent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
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
    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
    
    System::GUI()->renderGUI(cmdBuffer);
    
    vkCmdEndRenderPass(cmdBuffer);
}

void ScreenSpacePipeline::setFrame(Frame *frame) { m_pFrame = frame; }

Renderpass* ScreenSpacePipeline::getRenderpass() { return m_pRenderpass; }



//  Copyright © 2021 Subph. All rights reserved.
//

#include "pipeline.hpp"

#include "../system.hpp"

Pipeline::~Pipeline() {}
Pipeline::Pipeline()  {}
void Pipeline::cleanup() { m_cleaner.flush("Pipeline"); }

VkPipeline Pipeline::get() { return m_pipeline; }

void Pipeline::setRenderpass(VkRenderPass renderpass) { m_renderpass = renderpass; }
void Pipeline::setPipelineLayout(VkPipelineLayout pipelineLayout) { m_pipelineLayout = pipelineLayout; }
void Pipeline::setShaderStages(VECTOR<VkPipelineShaderStageCreateInfo> shaderStages) { m_shaderStages = shaderStages; }
void Pipeline::setVertexInputInfo(VkPipelineVertexInputStateCreateInfo vertexInputInfo) { m_vertexInputInfo = vertexInputInfo; }

void Pipeline::setupViewportInfo() {
    m_viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewportInfo.viewportCount = 1;
    m_viewportInfo.scissorCount  = 1;
}

void Pipeline::setupInputAssemblyInfo() {
    m_inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void Pipeline::setupRasterizationInfo() {
    m_rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    m_rasterizationInfo.cullMode    = VK_CULL_MODE_NONE;
    m_rasterizationInfo.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_rasterizationInfo.lineWidth   = 1.0f;
}

void Pipeline::setupMultisampleInfo() {
    m_multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void Pipeline::disableBlendAttachment() {
    m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    m_colorBlendAttachment.blendEnable = VK_FALSE;
}

void Pipeline::enableBlendAttachment() {
    m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    m_colorBlendAttachment.blendEnable         = VK_TRUE;
    m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    m_colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    m_colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
}

void Pipeline::setupColorBlendInfo() {
    m_colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_colorBlendInfo.attachmentCount = 1;
    m_colorBlendInfo.pAttachments    = &m_colorBlendAttachment;
}

void Pipeline::setupDynamicInfo() {
    m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    m_dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    m_dynamicInfo.dynamicStateCount = UINT32(m_dynamicStates.size());
    m_dynamicInfo.pDynamicStates    = m_dynamicStates.data();
}

void Pipeline::setupDepthStencilInfo() {
    m_depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_depthStencilInfo.depthTestEnable       = VK_TRUE;
    m_depthStencilInfo.depthWriteEnable      = VK_TRUE;
    m_depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    m_depthStencilInfo.stencilTestEnable     = VK_FALSE;
    m_depthStencilInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
}

void Pipeline::createGraphicsPipeline() {
    VkDevice device = System::Device()->getDevice();
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout     = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderpass;
    pipelineInfo.subpass    = 0;
    pipelineInfo.stageCount = UINT32(m_shaderStages.size());
    pipelineInfo.pStages             = m_shaderStages.data();
    pipelineInfo.pVertexInputState   = &m_vertexInputInfo;
    pipelineInfo.pViewportState      = &m_viewportInfo;
    pipelineInfo.pInputAssemblyState = &m_inputAssemblyInfo;
    pipelineInfo.pRasterizationState = &m_rasterizationInfo;
    pipelineInfo.pMultisampleState   = &m_multisampleInfo;
    pipelineInfo.pColorBlendState    = &m_colorBlendInfo;
    pipelineInfo.pDynamicState       = &m_dynamicInfo;
    pipelineInfo.pDepthStencilState  = &m_depthStencilInfo;
    
    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline );
    CHECK_VKRESULT(result, "failed to create graphics pipeline!");
    m_cleaner.push([=](){ vkDestroyPipeline(device, m_pipeline, nullptr); });
}

void Pipeline::createComputePipeline() {
    VkDevice device = System::Device()->getDevice();
    
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.stage  = m_shaderStages[0];
    
    VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline );
    CHECK_VKRESULT(result, "failed to create graphics pipeline!");
    m_cleaner.push([=](){ vkDestroyPipeline(device, m_pipeline, nullptr); });
}



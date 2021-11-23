//  Copyright © 2021 Subph. All rights reserved.
//

#include "main_pipeline.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

MainPipeline::~MainPipeline() {}
MainPipeline::MainPipeline() : m_pDevice(System::Device()) {}

void MainPipeline::cleanup() { m_cleaner.flush("InterferencePipeline"); }

void MainPipeline::setupShader() {
    LOG("MainPipeline::setupShader");
    Shader* vertShader = new Shader("resources/spirv/main1d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader("resources/spirv/main1d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); });
}

void MainPipeline::setupInput() {
    
}

void MainPipeline::createDescriptor() {
    LOG("MainPipeline::createDescriptor");
}

void MainPipeline::createPipelineLayout() {
    LOG("MainPipeline::createPipelineLayout");
    VkDevice device = m_pDevice->getDevice();
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
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

    VECTOR<VkDynamicState> dynamicStates;
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

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
    pipelineInfo.stageCount = 2;
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

void MainPipeline::createRenderpass() {
    VkSurfaceFormatKHR surfaceFormat = m_pDevice->getSurfaceFormat();
    m_pRenderpass = new Renderpass();
    m_pRenderpass->setupColorAttachment(surfaceFormat.format);
    m_pRenderpass->setup();
    m_pRenderpass->create();
    m_cleaner.push([=](){ m_pRenderpass->cleanup(); });
}

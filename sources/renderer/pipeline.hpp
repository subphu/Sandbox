//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"

class Pipeline {
    
public:
    Pipeline();
    ~Pipeline();
    
    void cleanup();
    
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
    VkPipelineVertexInputStateCreateInfo    m_vertexInputInfo;
    
    VkPipelineViewportStateCreateInfo       m_viewportInfo{};
    VkPipelineInputAssemblyStateCreateInfo  m_inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo  m_rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo    m_multisampleInfo{};
    
    VkPipelineColorBlendAttachmentState     m_colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo     m_colorBlendInfo{};
    
    VECTOR<VkDynamicState>                  m_dynamicStates;
    VkPipelineDynamicStateCreateInfo        m_dynamicInfo{};
    VkPipelineDepthStencilStateCreateInfo   m_depthStencilInfo{};
    
    void setRenderpass(VkRenderPass renderpass);
    void setPipelineLayout(VkPipelineLayout pipelineLayout);
    void setShaderStages(VECTOR<VkPipelineShaderStageCreateInfo> shaderStages);
    void setVertexInputInfo(VkPipelineVertexInputStateCreateInfo vertexInputInfo);
    
    void setupViewportInfo();
    void setupInputAssemblyInfo();
    void setupRasterizationInfo();
    void setupMultisampleInfo();
    
    void disableBlendAttachment();
    void enableBlendAttachment();
    void setupColorBlendInfo();
    
    // Optional
    void setupDynamicInfo();
    void setupDepthStencilInfo();

    void createComputePipeline();
    void createGraphicsPipeline();
    
    VkPipeline get();
    
private:
    Cleaner m_cleaner;
    
    VkRenderPass m_renderpass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};

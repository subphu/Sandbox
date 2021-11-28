//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/device.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/renderpass.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/buffer.hpp"
#include "../resources/frame.hpp"
#include "../resources/mesh.hpp"
#include "../resources/camera.hpp"

struct CameraMatrix {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Misc {
    glm::vec3 viewPosition;
    uint sampleSize;
};

class MainPipeline {
    
    
public:
    ~MainPipeline();
    MainPipeline();
    
    void cleanup();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput(uint sampleSize);
    void updateCameraInput(Camera* pCamera);
    void updateInterferenceInput(Buffer* pInterferenceBuffer);
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    void createFrame(UInt2D size);
    
    Frame* getFrame();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Pipeline* m_pPipeline;
    Renderpass* m_pRenderpass;
    Descriptor* m_pDescriptor;
    
    Buffer* m_pMiscBuffer;
    Buffer* m_pCameraBuffer;
    Buffer* m_pInterferenceBuffer;
    Frame*  m_pFrame;
    Mesh*   m_pSphere;
    VECTOR<Image*> m_pTextures;
    
    Misc         m_misc{};
    CameraMatrix m_cameraMatrix{};
    
    VkViewport m_viewport{};
    VkRect2D   m_scissor{};
    
    uint textureIdx = 6; // 3,4,
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
    
    void updateViewportScissor();
    
private:
    const uint USED_TEXTURE = 5;
    const std::vector<std::string> TEXTURE_NAMES = {"cliffrockface", "cobblestylized", "greasypan", "layered-rock1", "limestone6",  "roughrockface", "rustediron", "slimy-slippery-rock1", "slipperystonework", "worn-wet-old-cobblestone"};
    const std::string TEXTURE_ALBEDO_PATH    = "_albedo.png";
    const std::string TEXTURE_AO_PATH        = "_ao.png";
    const std::string TEXTURE_METALLIC_PATH  = "_metallic.png";
    const std::string TEXTURE_NORMAL_PATH    = "_normal.png";
    const std::string TEXTURE_ROUGHNESS_PATH = "_roughness.png";
    
    std::string getTextureName();
    std::string getAlbedoTexturePath();
    std::string getAOTexturePath();
    std::string getMetallicTexturePath();
    std::string getNormalTexturePath();
    std::string getRoughnessTexturePath();
    VECTOR<std::string> getPBRTexturePaths();
    
};

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


class GraphicsScene {
    
    struct PCMisc {
        glm::mat4 model;
        glm::vec3 viewPosition;
        uint isLight;
    };
    
    struct UBCamera {
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct UBLights {
        glm::vec4 color;
        glm::vec4 position[4];
        uint total = 4;
        float radiance;
    };
    
    struct UBParam {
        glm::vec4 albedo;
        float metallic   = 1.0;
        float roughness  = 0.0;
        float ao         = 1.0;
        uint  useTexture = 0;
        uint  useFluid   = 1;
        
        uint  interference     = 1;
        uint  phaseShift       = 0;
        float thicknessScale   = 0.3;
        float refractiveIndex  = 1.5;
        float reflectanceValue = 0.5;
        float opdOffset        = 0.;
        uint  opdSample        = 16384;
    };
    
    
public:
    ~GraphicsScene();
    GraphicsScene();
    
    void cleanup();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupInput();
    void updateTexture();
    void updateCubemap(Image* cubemap, Image* envMap, Image* reflMap, Image* brdfMap);
    void updateLightInput();
    void updateParamInput();
    void updateCameraInput(Camera* pCamera);
    void updateInterferenceInput(Image* pInterferenceImage);
    void updateHeightmapInput(Image* pHeightmapImage);
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    void createFrame(UInt2D size);
    void recreateFrame(UInt2D size);
    
    Frame* getFrame();
    Mesh * getMesh();
    Buffer* getMarkBuffer();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Pipeline* m_pMeshPipeline;
    Pipeline* m_pCubemapPipeline;
    Renderpass* m_pRenderpass;
    Descriptor* m_pDescriptor;
    
    Buffer* m_pLightBuffer;
    Buffer* m_pParamBuffer;
    Buffer* m_pCameraBuffer;
    Buffer* m_pMarkBuffer;
    Frame*  m_pFrame;
    
    Mesh*   m_pCube;
    VECTOR<Mesh*> m_pMesh;
    Image*  m_pCubemap;
    Image*  m_pEnvMap;
    Image*  m_pReflMap;
    Image*  m_pBrdfMap;
    Image*  m_pHeightmap;
    Image*  m_pInterference;
    VECTOR<Image*> m_pTextures;
    
    PCMisc   m_misc{};
    UBLights m_lights{};
    UBCamera m_camera{};
    UBParam  m_param{};
    
    VkViewport m_viewport{};
    VkRect2D   m_scissor{};
    
    uint m_textureIdx = 6; // 3,4,
    long m_iteration = 0;
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
    
    void updateViewportScissor();
    
};

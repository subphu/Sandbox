//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "../renderer/pipeline.hpp"
#include "../renderer/renderpass.hpp"
#include "../renderer/descriptor.hpp"
#include "../resources/buffer.hpp"
#include "../resources/frame.hpp"
#include "../resources/mesh.hpp"
#include "../resources/camera.hpp"


class GraphicsEquirect {
    
    struct PCMisc {
        glm::mat4 mvp;
        int layer;
    };
    
    
public:
    ~GraphicsEquirect();
    GraphicsEquirect();
    
    void cleanup();
    void render();
    void render(VkCommandBuffer cmdBuffer);
    
    void setupShader();
    void setupMesh();
    void setupInput(Image* image);
    void cleanFrame();
    
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    void createRenderpass();
    void createFrame(uint32_t size);
    
    Image* copyFrameImage();
    
private:
    Cleaner m_cleaner;
    Pipeline* m_pPipeline;
    Renderpass* m_pRenderpass;
    Descriptor* m_pDescriptor;
    
    Frame* m_pFrame;
    
    Mesh*  m_pCube;
    Image* m_pEquirect;
    
    PCMisc m_misc{};
    
    VkViewport m_viewport{};
    VkRect2D   m_scissor{};
    
    VkPipelineLayout m_pipelineLayout;
    
    VkPushConstantRange m_pushConstantRange;
    VECTOR<VkPipelineShaderStageCreateInfo> m_shaderStages;
    
    void updateViewportScissor();
    
private:
    const glm::mat4 CUBEMAP_PROJ = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
    const glm::mat4 CUBEMAP_VIEWS[6] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
};

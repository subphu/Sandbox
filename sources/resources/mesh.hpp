//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "buffer.hpp"

class Mesh {
    
public:
    Mesh();
    ~Mesh();
    
    void cleanup();
    
    void createPlane();
    void createQuad();
    void createCube();
    void createSphere(int wedge = 50, int segment = 50);
    void loadModel(const char* filename);
    
    void scale(glm::vec3 size);
    void rotate(float angle, glm::vec3 axis);
    void translate(glm::vec3 translation);
    
    void createIndexBuffer();
    void createVertexBuffer();
    void createVertexStateInfo();
    
    uint32_t sizeofPositions();
    uint32_t sizeofNormals();
    uint32_t sizeofTexCoords();
    uint32_t sizeofIndices();
    
    glm::mat4 getMatrix();
    VkPipelineVertexInputStateCreateInfo getVertexStateInfo();
    
private:
    Cleaner m_cleaner;
    Device* m_pDevice;
    Buffer* m_vertexBuffer;
    Buffer* m_indexBuffer;
    
    glm::mat4 m_model;
    
    VECTOR<VkVertexInputAttributeDescription> m_vertexAttrDescs;
    VkPipelineVertexInputStateCreateInfo m_vertexStateInfo{};
    
    VECTOR<glm::vec3> m_positions;
    VECTOR<glm::vec3> m_normals;
    VECTOR<glm::vec2> m_texCoords;
    VECTOR<uint32_t>  m_indices;
    
    const uint32_t m_sizeofPosition = sizeof(glm::vec3);
    const uint32_t m_sizeofNormal   = sizeof(glm::vec3);
    const uint32_t m_sizeofTexCoord = sizeof(glm::vec2);
    const uint32_t m_sizeofIndex    = sizeof(uint32_t);
    
};

//  Copyright © 2021 Subph. All rights reserved.
//

#include "graphics_scene.hpp"

#include "../system.hpp"
#include "../resources/shader.hpp"

GraphicsScene::~GraphicsScene() {}
GraphicsScene::GraphicsScene() : m_pDevice(System::Device()) {}

void GraphicsScene::cleanup() { m_cleaner.flush("GraphicsScene"); }

void GraphicsScene::render(VkCommandBuffer cmdBuffer) {
    Settings* settings = System::Settings();
    VkPipelineLayout pipelineLayout  = m_pipelineLayout;
    VkPipeline       meshPipeline    = m_pMeshPipeline->get();
    VkPipeline       cubemapPipeline = m_pCubemapPipeline->get();
    VkRenderPass     renderpass      = m_pRenderpass->get();
    VkFramebuffer    framebuffer     = m_pFrame->getFramebuffer();
    VkRect2D         scissor         = m_scissor;
    VkViewport       viewport        = m_viewport;
    Mesh *mesh = m_pMesh[settings->Shapes];
    
    VkDeviceSize offsets  = 0;
    VkBuffer meshVertexBuffer = mesh->getVertexBuffer()->get();
    VkBuffer meshIndexBuffer  = mesh->getIndexBuffer()->get();
    uint32_t meshIndexSize    = mesh->getIndexSize();
    VkBuffer cubeVertexBuffer = m_pCube->getVertexBuffer()->get();
    VkBuffer cubeIndexBuffer  = m_pCube->getIndexBuffer()->get();
    uint32_t cubeIndexSize    = m_pCube->getIndexSize();
    
    
    VkDescriptorSet cameraDescSet  = m_pDescriptor->getDescriptorSet(S0);
    VkDescriptorSet miscDescSet = m_pDescriptor->getDescriptorSet(S1);
    VkDescriptorSet textureDescSet = m_pDescriptor->getDescriptorSet(S2);
    VkDescriptorSet heightmapDescSet = m_pDescriptor->getDescriptorSet(S3);
    VkDescriptorSet interferenceDescSet = m_pDescriptor->getDescriptorSet(S4);
    VkDescriptorSet cubemapDescSet = m_pDescriptor->getDescriptorSet(S5);
    
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
    
    Buffer* pMarkBuffer = m_pMarkBuffer;
    pMarkBuffer->cmdClearBuffer(cmdBuffer, 0.5);
    
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
    vkCmdBeginRenderPass(cmdBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipeline);
    
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S0, 1, &cameraDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S5, 1, &cubemapDescSet, 0, nullptr);
    
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &cubeVertexBuffer, &offsets);
    vkCmdBindIndexBuffer  (cmdBuffer, cubeIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(cmdBuffer, cubeIndexSize, 1, 0, 0, 0);
    
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPipeline);
    
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S0, 1, &cameraDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S1, 1, &miscDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S2, 1, &textureDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S3, 1, &heightmapDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S4, 1, &interferenceDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S5, 1, &cubemapDescSet, 0, nullptr);
    
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &meshVertexBuffer, &offsets);
    vkCmdBindIndexBuffer  (cmdBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    m_misc.model = mesh->getMatrix();
    m_misc.isLight = 0;
    vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PCMisc), &m_misc);
    
    vkCmdDrawIndexed(cmdBuffer, meshIndexSize, 1, 0, 0, 0);
    
    m_misc.isLight = 1;
    for (int i = 0; i < m_lights.total; i++) {
        m_misc.model = glm::translate(glm::mat4(1.0), glm::vec3(m_lights.position[i]));
        m_misc.model = glm::scale(m_misc.model, glm::vec3(0.2));
        vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PCMisc), &m_misc);
        
        vkCmdDrawIndexed(cmdBuffer, meshIndexSize, 1, 0, 0, 0);
    }
    
    vkCmdEndRenderPass(cmdBuffer);
}

void GraphicsScene::setupShader() {
    LOG("GraphicsScene::setupShader");
    Shader* vertShader = new Shader(SPIRV_PATH + "main1d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* fragShader = new Shader(SPIRV_PATH + "main1d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    Shader* cubeVertShader = new Shader(SPIRV_PATH + "cubemap.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    Shader* cubeFratShader = new Shader(SPIRV_PATH + "cubemap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    m_shaderStages = { vertShader->getShaderStageInfo(), fragShader->getShaderStageInfo(), cubeVertShader->getShaderStageInfo(), cubeFratShader->getShaderStageInfo() };
    m_cleaner.push([=](){ vertShader->cleanup(); fragShader->cleanup(); cubeVertShader->cleanup(); cubeFratShader->cleanup(); });
}

void GraphicsScene::setupInput() {
    LOG("GraphicsScene::setupInput");
    m_lights.total = System::Settings()->TotalLight;
    
    m_pCameraBuffer = new Buffer();
    m_pCameraBuffer->setup(sizeof(UBCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pCameraBuffer->create();
    m_cleaner.push([=](){ m_pCameraBuffer->cleanup(); });
    
    m_pLightBuffer = new Buffer();
    m_pLightBuffer->setup(sizeof(UBLights), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pLightBuffer->create();
    m_cleaner.push([=](){ m_pLightBuffer->cleanup(); });
    
    m_pParamBuffer = new Buffer();
    m_pParamBuffer->setup(sizeof(UBParam), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pParamBuffer->create();
    m_cleaner.push([=](){ m_pParamBuffer->cleanup(); });
    
    m_pDescriptor->setupPointerBuffer(S0, B0, m_pCameraBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerBuffer(S1, B0, m_pLightBuffer->getDescriptorInfo());
    m_pDescriptor->setupPointerBuffer(S1, B1, m_pParamBuffer->getDescriptorInfo());
    
    m_pDescriptor->update(S0);
    m_pDescriptor->update(S1);
    
    Mesh* cube = new Mesh();
    cube->createCube();
    cube->createVertexBuffer();
    cube->createIndexBuffer();
    cube->createVertexStateInfo();
    m_cleaner.push([=](){ cube->cleanup(); });
    
    Mesh* sphere = new Mesh();
    sphere->createSphere(200, 200);
    sphere->createVertexBuffer();
    sphere->createIndexBuffer();
    sphere->createVertexStateInfo();
    m_cleaner.push([=](){ sphere->cleanup(); });
    
    Mesh* model = new Mesh();
    model->loadModel((MODEL_PATH + "bunny/bunny.obj").c_str());
    model->createVertexBuffer();
    model->createIndexBuffer();
    model->createVertexStateInfo();
    model->translate({.2, -.6, 0.});
    m_cleaner.push([=](){ model->cleanup(); });
    
    m_pMesh.push_back(sphere);
    m_pMesh.push_back(cube);
    m_pMesh.push_back(model);
    m_pCube = cube;
}

void GraphicsScene::updateTexture() {
    System::Device()->waitIdle();
    VECTOR<STRING> pbrPaths = System::Files()->getTexturePBRPaths();
    bool hasImage = m_pTextures.size() > 0;
    m_pTextures.resize(pbrPaths.size());
    for (uint i = 0; i < pbrPaths.size(); i++) {
        if (hasImage) m_pTextures[i]->cleanup();
        m_pTextures[i] = new Image();
        m_pTextures[i]->setupForTexture(pbrPaths[i]);
        m_pTextures[i]->createWithSampler();
        m_pTextures[i]->cmdCopyRawDataToImage();
        m_pTextures[i]->cmdTransitionToShaderR();
        m_cleaner.push([=](){ m_pTextures[i]->cleanup(); });
        m_pDescriptor->setupPointerImage(S2, i, m_pTextures[i]->getDescriptorInfo());
    }
    m_pDescriptor->update(S2);
}

void GraphicsScene::updateCubemap(Image* cubemap, Image* envMap, Image* reflMap, Image* brdfMap) {
    m_pCubemap = cubemap;
    m_pEnvMap = envMap;
    m_pReflMap = reflMap;
    m_pBrdfMap = brdfMap;
    m_pCubemap->cmdTransitionToShaderR();
    m_pEnvMap->cmdTransitionToShaderR();
    m_pReflMap->cmdTransitionToShaderR();
    m_pBrdfMap->cmdTransitionToShaderR();
    m_pDescriptor->setupPointerImage(S5, B0, m_pCubemap->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S5, B1, m_pEnvMap->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S5, B2, m_pReflMap->getDescriptorInfo());
    m_pDescriptor->setupPointerImage(S5, B3, m_pBrdfMap->getDescriptorInfo());
    m_pDescriptor->update(S5);
    m_cleaner.push([=](){ m_pCubemap->cleanup(); });
    m_cleaner.push([=](){ m_pEnvMap->cleanup(); });
    m_cleaner.push([=](){ m_pReflMap->cleanup(); });
    m_cleaner.push([=](){ m_pBrdfMap->cleanup(); });
}

void GraphicsScene::updateLightInput() {
    Settings* settings = System::Settings();
    m_lights.radiance = settings->Radiance;
    m_lights.total = settings->TotalLight;
    m_lights.color = settings->LightColor;
    glm::vec2 distance = settings->Distance;
    m_iteration = settings->LightMove ? settings->Iteration : m_iteration;
    float interval = glm::radians(360.f/m_lights.total);
    for (int i = 0; i < m_lights.total; i++) {
        m_lights.position[i].z = distance.x;
        m_lights.position[i].x = sin(m_iteration / 100.f + i * interval) * distance.y;
        m_lights.position[i].y = cos(m_iteration / 100.f + i * interval) * distance.y;
    }
    m_pLightBuffer->fillBuffer(&m_lights, sizeof(UBLights));
}

void GraphicsScene::updateParamInput() {
    Settings* settings = System::Settings();
    m_param.albedo     = settings->Albedo;
    m_param.metallic   = settings->Metallic;
    m_param.roughness  = settings->Roughness;
    m_param.ao         = settings->AO;
    m_param.useTexture = settings->UseTexture;
    m_param.useFluid   = settings->UseFluid;
    m_param.interference     = settings->Interference;
    m_param.phaseShift       = settings->PhaseShift;
    m_param.thicknessScale   = settings->ThicknessScale;
    m_param.refractiveIndex  = settings->RefractiveIndex;
    m_param.reflectanceValue = settings->ReflectanceValue;
    m_param.opdSample        = settings->OPDSample;
    m_pParamBuffer->fillBuffer(&m_param, sizeof(UBParam));
}

void GraphicsScene::updateCameraInput(Camera* pCamera) {
    UInt2D size = m_pFrame->getSize();
    m_misc.viewPosition = pCamera->getPosition();
    m_camera.view = pCamera->getViewMatrix();
    m_camera.proj = pCamera->getProjection((float) size.width / size.height);
    m_pCameraBuffer->fillBuffer(&m_camera, sizeof(UBCamera));
}

void GraphicsScene::updateHeightmapInput(Image *pHeightmapImage) {
    m_pHeightmap = pHeightmapImage;
    m_pHeightmap->cmdTransitionToShaderR();
    m_pDescriptor->setupPointerImage(S3, B0, m_pHeightmap->getDescriptorInfo());
    m_pDescriptor->update(S3);
}

void GraphicsScene::updateInterferenceInput(Image* pInterferenceImage) {
    m_pInterference = pInterferenceImage;
    m_pInterference->cmdTransitionToShaderR();
    m_pDescriptor->setupPointerImage(S4, B0, m_pInterference->getDescriptorInfo());
    
    uint bufferSize = m_pInterference->getImageSize().width * sizeof(float);
    m_pMarkBuffer = new Buffer();
    m_pMarkBuffer->setup(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    m_pMarkBuffer->create();
    m_pDescriptor->setupPointerBuffer(S4, B1, m_pMarkBuffer->getDescriptorInfo());
    
    m_pDescriptor->update(S4);
}

void GraphicsScene::createDescriptor() {
    LOG("GraphicsScene::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(S0);
    m_pDescriptor->addLayoutBindings(S0, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_VERTEX_BIT);
    m_pDescriptor->createLayout(S0);
    
    m_pDescriptor->setupLayout(S1);
    m_pDescriptor->addLayoutBindings(S1, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S1, B1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S1);
    
    m_pDescriptor->setupLayout(S2);
    for (uint i = 0; i < 5; i++) {
        m_pDescriptor->addLayoutBindings(S2, i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                       VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    m_pDescriptor->createLayout(S2);
    
    m_pDescriptor->setupLayout(S3);
    m_pDescriptor->addLayoutBindings(S3, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S3);
    
    m_pDescriptor->setupLayout(S4);
    m_pDescriptor->addLayoutBindings(S4, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S4, B1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S4);
    
    m_pDescriptor->setupLayout(S5);
    m_pDescriptor->addLayoutBindings(S5, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S5, B1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S5, B2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->addLayoutBindings(S5, B3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(S5);
    
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(S0);
    m_pDescriptor->allocate(S1);
    m_pDescriptor->allocate(S2);
    m_pDescriptor->allocate(S3);
    m_pDescriptor->allocate(S4);
    m_pDescriptor->allocate(S5);
    m_cleaner.push([=](){ m_pDescriptor->cleanup(); });
}

void GraphicsScene::createRenderpass() {
    m_pRenderpass = new Renderpass();
    m_pRenderpass->setupColorAttachment();
    m_pRenderpass->setupDepthAttachment();
    m_pRenderpass->setup();
    m_pRenderpass->create();
    m_cleaner.push([=](){ m_pRenderpass->cleanup(); });
}

void GraphicsScene::createPipelineLayout() {
    LOG("GraphicsScene::createPipelineLayout");
    VkDevice device = m_pDevice->getDevice();
    VECTOR<VkDescriptorSetLayout> descSetLayouts = {
        m_pDescriptor->getDescriptorLayout(S0),
        m_pDescriptor->getDescriptorLayout(S1),
        m_pDescriptor->getDescriptorLayout(S2),
        m_pDescriptor->getDescriptorLayout(S3),
        m_pDescriptor->getDescriptorLayout(S4),
        m_pDescriptor->getDescriptorLayout(S5)
    };
    
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(PCMisc);
    pushConstantRange.offset = 0;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = UINT32(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts    = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
    
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    m_cleaner.push([=](){ vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr); });
}

void GraphicsScene::createPipeline() {
    LOG("GraphicsScene::createPipeline");
    VkRenderPass renderpass = m_pRenderpass->get();
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VECTOR<VkPipelineShaderStageCreateInfo> shaderStages = m_shaderStages;
    VkPipelineVertexInputStateCreateInfo cubeVertexInfo = m_pCube->getVertexStateInfo();
    VkPipelineVertexInputStateCreateInfo meshVertexInfo = m_pCube->getVertexStateInfo();
    
    m_pMeshPipeline = new Pipeline();
    m_pMeshPipeline->setRenderpass(renderpass);
    m_pMeshPipeline->setPipelineLayout(pipelineLayout);
    m_pMeshPipeline->setShaderStages({shaderStages[0], shaderStages[1]});
    m_pMeshPipeline->setVertexInputInfo(meshVertexInfo);
    
    m_pMeshPipeline->setupViewportInfo();
    m_pMeshPipeline->setupInputAssemblyInfo();
    m_pMeshPipeline->setupRasterizationInfo();
    m_pMeshPipeline->setupMultisampleInfo();
    
    m_pMeshPipeline->setupBlendAttachment();
    m_pMeshPipeline->setupColorBlendInfo();
    
    m_pMeshPipeline->setupDynamicInfo();
    m_pMeshPipeline->setupDepthStencilInfo();

    m_pMeshPipeline->createGraphicsPipeline();
    m_cleaner.push([=](){ m_pMeshPipeline->cleanup(); });
    
    m_pCubemapPipeline = new Pipeline();
    m_pCubemapPipeline->setRenderpass(renderpass);
    m_pCubemapPipeline->setPipelineLayout(pipelineLayout);
    m_pCubemapPipeline->setShaderStages({shaderStages[2], shaderStages[3]});
    m_pCubemapPipeline->setVertexInputInfo(cubeVertexInfo);
    
    m_pCubemapPipeline->setupViewportInfo();
    m_pCubemapPipeline->setupInputAssemblyInfo();
    m_pCubemapPipeline->setupRasterizationInfo();
    m_pCubemapPipeline->setupMultisampleInfo();
    
    m_pCubemapPipeline->setupBlendAttachment(VK_FALSE);
    m_pCubemapPipeline->setupColorBlendInfo();
    
    m_pCubemapPipeline->setupDynamicInfo();
    m_pCubemapPipeline->setupDepthStencilInfo(VK_FALSE);

    m_pCubemapPipeline->createGraphicsPipeline();
    m_cleaner.push([=](){ m_pCubemapPipeline->cleanup(); });
}

void GraphicsScene::createFrame(UInt2D size) {
    LOG("GraphicsScene::createFrame");
    m_pFrame = new Frame(size);
    m_pFrame->createImageResource();
    m_pFrame->createDepthResource();
    m_pFrame->createFramebuffer(m_pRenderpass);
    m_cleaner.push([=](){ m_pFrame->cleanup(); });
    updateViewportScissor();
}

void GraphicsScene::recreateFrame(UInt2D size) {
    LOG("GraphicsScene::recreateFrame");
    m_pFrame->cleanup();
    m_pFrame->setSize(size);
    m_pFrame->createImageResource();
    m_pFrame->createDepthResource();
    m_pFrame->createFramebuffer(m_pRenderpass);
    updateViewportScissor();
}

void GraphicsScene::updateViewportScissor() {
    UInt2D extent = m_pFrame->getSize();
    m_viewport.x = 0.f;
    m_viewport.y = 0.f;
    m_viewport.width  = extent.width;
    m_viewport.height = extent.height;
    m_viewport.minDepth = 0.f;
    m_viewport.maxDepth = 1.f;
    m_scissor.offset = {0, 0};
    m_scissor.extent = extent;
}

Frame* GraphicsScene::getFrame() { return m_pFrame; }
Mesh * GraphicsScene::getMesh () { return m_pMesh[System::Settings()->Shapes]; }


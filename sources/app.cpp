//  Copyright © 2021 Subph. All rights reserved.
//

#include "app.hpp"
#include "include.h"
#include "system.hpp"

void App::run() {
    setup();
    loop();
    cleanup();
}

void App::cleanup() { m_cleaner.flush("App"); }

void App::initWindow() {
    LOG("App::initWindow");
    m_pWindow = new Window();
    m_pWindow->create({1200, 800}, "Vulkan");
    m_pWindow->setWindowPosition(0, 0);
    m_pWindow->enableInput();
    m_cleaner.push([=](){ m_pWindow->close(); });
}

void App::initDevice() {
    LOG("App::initDevice");
    Window* pWindow = m_pWindow;
    m_pDevice = new Device();
    m_pDevice->setup();
    m_pDevice->createInstance();
    m_pDevice->createDebugMessenger();
    m_pDevice->createSurface(pWindow->getGLFWwindow());
    m_pDevice->selectPhysicalDevice();
    m_pDevice->createLogicalDevice();
    System::Instance().setDevice(m_pDevice);
    m_cleaner.push([=](){ m_pDevice->cleanup(); });
}

void App::initCommander() {
    LOG("App::initCommander");
    m_pCommander = new Commander();
    m_pCommander->setupPool();
    m_pCommander->createPool();
    System::Instance().setCommander(m_pCommander);
    m_cleaner.push([=](){ m_pCommander->cleanup(); });
}

void App::createGraphicsScreen() {
    LOG("App::createGraphicsScreen");
    m_pGraphicsScreen = new GraphicsScreen();
    m_pGraphicsScreen->setupShader();
    m_pGraphicsScreen->createDescriptor();
    m_pGraphicsScreen->createRenderpass();
    m_pGraphicsScreen->createPipelineLayout();
    m_pGraphicsScreen->createPipeline();
    m_cleaner.push([=](){ m_pGraphicsScreen->cleanup(); });
}

void App::createSwapchain() {
    LOG("App::createSwapchain");
    Renderpass* pRenderpass = m_pGraphicsScreen->getRenderpass();
    m_pSwapchain = new Swapchain();
    m_pSwapchain->setup();
    m_pSwapchain->create();
    m_pSwapchain->createFrames(pRenderpass);
    m_cleaner.push([=](){ m_pSwapchain->cleanup(); });
}

void App::createGUI() {
    LOG("App::createGUI");
    Renderpass* pRenderpass  = m_pGraphicsScreen->getRenderpass();
    m_pGUI = new GUI();
    m_pGUI->initGUI(m_pWindow, pRenderpass);
    m_cleaner.push([=](){ m_pGUI->cleanupGUI(); });
}

void App::createGraphicsScene() {
    LOG("App::createGraphicsScene");
    UInt2D size = m_pWindow->getFrameSize();
    m_pGraphicsScene = new GraphicsScene();
    m_pGraphicsScene->setupShader();
    m_pGraphicsScene->createDescriptor();
    m_pGraphicsScene->setupInput();
    m_pGraphicsScene->createRenderpass();
    m_pGraphicsScene->createPipelineLayout();
    m_pGraphicsScene->createPipeline();
    m_pGraphicsScene->createFrame(size);
    m_cleaner.push([=](){ m_pGraphicsScene->cleanup(); });
    
    m_pGraphicsScreen->setupInput(m_pGraphicsScene->getFrame());
}

void App::createComputeFluid() {
    LOG("App::createComputeFluid");
    m_pComputeFluid = new ComputeFluid();
    m_pComputeFluid->setupShader();
    m_pComputeFluid->createDescriptor();
    m_pComputeFluid->setupInput();
    m_pComputeFluid->setupOutput();
    m_pComputeFluid->createPipelineLayout();
    m_pComputeFluid->createPipeline();
    m_cleaner.push([=](){ m_pComputeFluid->cleanup(); });
    
    m_pGraphicsScene->updateHeightmapInput(m_pComputeFluid->getHeightImage());
    m_pGUI->updateFluidImage(m_pComputeFluid->getFluidImage());
    m_pGUI->updateHeightMapImage(m_pComputeFluid->getHeightImage());
    m_pGUI->updateIridescentImage(m_pComputeFluid->getIridescentImage());
}

void App::createInterference() {
    LOG("App::createInterference");
    ComputeInterference*  pComputeInterference = new ComputeInterference();
    pComputeInterference->setupShader();
    pComputeInterference->createDescriptor();
    pComputeInterference->setupInput();
    pComputeInterference->setupOutput();
    pComputeInterference->createPipelineLayout();
    pComputeInterference->createPipeline();
    pComputeInterference->dispatch();
    
    Image* interferenceImage = pComputeInterference->copyOutputImage();
    m_cleaner.push([=](){ interferenceImage->cleanup(); });
    pComputeInterference->cleanup();
    
    m_pComputeFluid->updateInterferenceInput(interferenceImage);
    m_pGraphicsScene->updateInterferenceInput(interferenceImage);
    m_pGUI->addInterferenceImage(interferenceImage);
}

void App::createCubemap() {
    LOG("App::createGraphicsEquirect");
    Files *pFiles = System::Files();
    Image *hdrImg, *hdrEnv, *cubemap, *envMap, *reflMap, *brdfMap;
    ComputeHDR* pComputeHDR = new ComputeHDR();
    pComputeHDR->setupShader();
    pComputeHDR->createDescriptor();
    pComputeHDR->createPipelineLayout();
    pComputeHDR->createPipeline();
    
    pComputeHDR->setupInputOutput(pFiles->getCubemapHDRPath());
    hdrImg = pComputeHDR->dispatch();
    
    pComputeHDR->cleanInputOutput();
    pComputeHDR->setupInputOutput(pFiles->getCubemapEnvPath());
    hdrEnv = pComputeHDR->dispatch();
    pComputeHDR->cleanup();
    
    uint length = 1024;
    GraphicsEquirect* pGraphicsEquirect = new GraphicsEquirect();
    pGraphicsEquirect->setupShader();
    pGraphicsEquirect->createDescriptor();
    pGraphicsEquirect->setupMesh();
    pGraphicsEquirect->createRenderpass();
    pGraphicsEquirect->createPipelineLayout();
    pGraphicsEquirect->createPipeline();
    
    pGraphicsEquirect->setupInput(hdrImg);
    pGraphicsEquirect->createFrame(length);
    cubemap = pGraphicsEquirect->render();
    m_cleaner.push([=](){ cubemap->cleanup(); });
    
    pGraphicsEquirect->cleanFrame();
    pGraphicsEquirect->setupInput(hdrEnv);
    pGraphicsEquirect->createFrame(length / 8);
    envMap = pGraphicsEquirect->render();
    m_cleaner.push([=](){ envMap->cleanup(); });
    
    pGraphicsEquirect->cleanup();
    hdrImg->cleanup();
    hdrEnv->cleanup();
    
    GraphicsReflection* pGraphicsReflection = new GraphicsReflection();
    pGraphicsReflection->setupShader();
    pGraphicsReflection->createDescriptor();
    pGraphicsReflection->setupMesh();
    pGraphicsReflection->createRenderpass();
    pGraphicsReflection->createPipelineLayout();
    pGraphicsReflection->createPipeline();
    
    pGraphicsReflection->setupInput(cubemap);
    pGraphicsReflection->createFrame();
    reflMap = pGraphicsReflection->render();
    m_cleaner.push([=](){ reflMap->cleanup(); });
    pGraphicsReflection->cleanup();
    
    ComputeBRDF* pComputeBRDF = new ComputeBRDF();
    pComputeBRDF->setupShader();
    pComputeBRDF->createDescriptor();
    pComputeBRDF->createPipelineLayout();
    pComputeBRDF->createPipeline();
    brdfMap = pComputeBRDF->dispatch({1024, 1024});
    m_cleaner.push([=](){ brdfMap->cleanup(); });
    pComputeBRDF->cleanup();
    
    m_pGraphicsScene->updateCubemap(cubemap, envMap, reflMap, brdfMap);
}

void App::setup() {
    System::Instance().initFiles();
    m_pCamera = new Camera();
    initWindow();
    initDevice();
    initCommander();
    createGraphicsScreen();
    createSwapchain();
    createGUI();
    
    createGraphicsScene();
    createComputeFluid();
    
    createInterference();
    createCubemap();
    
}

void App::draw() {
    Swapchain* pSwapchain = m_pSwapchain;
    GraphicsScene* pGraphicsScene = m_pGraphicsScene;
    ComputeFluid* pComputeFluid = m_pComputeFluid;
    GraphicsScreen* pGraphicsScreen = m_pGraphicsScreen;
    GUI* pGUI = m_pGUI;
    
    pSwapchain->prepareFrame();
    Frame*      pCurrentFrame = pSwapchain->getCurrentFrame();
    VkCommandBuffer cmdBuffer = pSwapchain->getCommandBuffer();
    
    VkCommandBufferBeginInfo commandBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkResult result = vkBeginCommandBuffer(cmdBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    
    if (System::Settings()->RunFluid) {
        pComputeFluid->dispatch(cmdBuffer);
    }
    
    pGraphicsScene->render(cmdBuffer);
    
    pGraphicsScreen->setFrame(pCurrentFrame);
    pGraphicsScreen->render(cmdBuffer, pGUI);
    
    vkEndCommandBuffer(cmdBuffer);
    
    pSwapchain->submitFrame();
    pSwapchain->presentFrame();
}

void App::update() {
    Settings* settings = System::Settings();
    Window*   pWindow  = m_pWindow;
    if (pWindow->getCursorPosition().x > 250 &&
        pWindow->getCursorPosition().x < pWindow->getSize().width - 250) {
        if (settings->LockFocus) moveViewLock(pWindow);
        else                     moveView(pWindow);
    }
    
    m_pGraphicsScene->updateLightInput();
    m_pGraphicsScene->updateCameraInput(m_pCamera);
    System::Settings()->CameraPos = m_pCamera->getPosition();
}

void App::moveView(Window* pWindow) {
    m_pCamera->setLockFocus(System::Settings()->LockFocus);
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += pWindow->getKeyState(key_d) - pWindow->getKeyState(key_a);
    movement.y += pWindow->getKeyState(key_e) - pWindow->getKeyState(key_q);
    movement.z += pWindow->getKeyState(key_w) - pWindow->getKeyState(key_s);
    m_pCamera->move(movement);
    
    if (pWindow->getMouseBtnState(mouse_btn_left)) {
        glm::vec2 delta = pWindow->getCursorOffset();
        m_pCamera->turn(delta * glm::vec2(-4.0, -4.0));
    }
}

void App::moveViewLock(Window* pWindow) {
    m_pCamera->setLockFocus(System::Settings()->LockFocus);
    float scale = m_pCamera->getDistance() * .5;
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += (pWindow->getKeyState(key_d) - pWindow->getKeyState(key_a)) * scale;
    movement.y += (pWindow->getKeyState(key_e) - pWindow->getKeyState(key_q)) * scale;
    movement.z += (pWindow->getKeyState(key_w) - pWindow->getKeyState(key_s)) * scale;
    movement.z += pWindow->getScrollOffset().y * scale;
    
    if (pWindow->getMouseBtnState(mouse_btn_left)) {
        scale = m_pCamera->getDistance() * .2;
        glm::vec2 cursorOffset = pWindow->getCursorOffset();
        movement.x += cursorOffset.x * scale;
        movement.y += cursorOffset.y * scale;
    }
    m_pCamera->move(movement);
}

void App::loop() {
    LOG("App::loop");
    RenderTime* pRenderTime = System::RenderTime();
    Settings  * pSettings   = System::Settings();
    
    while (m_pWindow->isOpen()) {
        bool lockFps = System::Settings()->LockFPS;
        
        pSettings->Iteration++;
        m_pWindow->pollEvents();
        checkResized();
        update();
        m_pWindow->resetInput();
        
        pRenderTime->startRender();
        while (pRenderTime->checkLag()) {
            draw();
            pRenderTime->addRenderTime();
            pRenderTime->sleepIf(lockFps);
        }
    }
    m_pDevice->waitIdle();
}

void App::checkResized() {
    if (!m_pWindow->checkResized()) return;
    LOG("App::resized");
    UInt2D size = m_pWindow->getFrameSize();
    
    m_pSwapchain->recreate();
    m_pGraphicsScene->recreateFrame(size);
    m_pGraphicsScreen->setupInput(m_pGraphicsScene->getFrame());
}

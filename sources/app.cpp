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

void App::createScreenSpacePipeline() {
    LOG("App::createScreenSpacePipeline");
    m_pScreenSpacePipeline = new ScreenSpacePipeline();
    m_pScreenSpacePipeline->setupShader();
    m_pScreenSpacePipeline->createDescriptor();
    m_pScreenSpacePipeline->createRenderpass();
    m_pScreenSpacePipeline->createPipelineLayout();
    m_pScreenSpacePipeline->createPipeline();
    m_cleaner.push([=](){ m_pScreenSpacePipeline->cleanup(); });
}

void App::createSwapchain() {
    LOG("App::createSwapchain");
    Renderpass* pRenderpass = m_pScreenSpacePipeline->getRenderpass();
    m_pSwapchain = new Swapchain();
    m_pSwapchain->setup();
    m_pSwapchain->create();
    m_pSwapchain->createFrames(pRenderpass);
    m_cleaner.push([=](){ m_pSwapchain->cleanup(); });
}

void App::createMainPipeline() {
    LOG("App::createMainPipeline");
    UInt2D size = m_pWindow->getFrameSize();
    m_pMainPipeline = new MainPipeline();
    m_pMainPipeline->setupShader();
    m_pMainPipeline->createDescriptor();
    m_pMainPipeline->setupInput();
    m_pMainPipeline->createRenderpass();
    m_pMainPipeline->createPipelineLayout();
    m_pMainPipeline->createPipeline();
    m_pMainPipeline->createFrame(size);
    m_cleaner.push([=](){ m_pMainPipeline->cleanup(); });
    
    m_pScreenSpacePipeline->setupInput(m_pMainPipeline->getFrame());
}

void App::createFluidPipeline() {
    LOG("App::createFluidPipeline");
    m_pFluidPipeline = new FluidPipeline();
    m_pFluidPipeline->setupShader();
    m_pFluidPipeline->createDescriptor();
    m_pFluidPipeline->setupInput();
    m_pFluidPipeline->setupOutput();
    m_pFluidPipeline->createPipelineLayout();
    m_pFluidPipeline->createPipeline();
    m_cleaner.push([=](){ m_pFluidPipeline->cleanup(); });
}

void App::createInterferencePipeline() {
    LOG("App::renderInterference");
    m_pInterferencePipeline = new InterferencePipeline();
    m_pInterferencePipeline->setupShader();
    m_pInterferencePipeline->createDescriptor();
    m_pInterferencePipeline->setupInput();
    m_pInterferencePipeline->setupOutput();
    m_pInterferencePipeline->createPipelineLayout();
    m_pInterferencePipeline->createPipeline();
    m_cleaner.push([=](){ m_pInterferencePipeline->cleanup(); });
}

void App::dispatchInterference() {
    VkCommandBuffer cmdBuffer = m_pCommander->createCommandBuffer();
    m_pCommander->beginSingleTimeCommands(cmdBuffer);
    m_pInterferencePipeline->dispatch(cmdBuffer);
    m_pCommander->endSingleTimeCommands(cmdBuffer);
    
    m_pFluidPipeline->updateInterferenceInput(m_pInterferencePipeline->getOutputBuffer());
    m_pMainPipeline->updateInterferenceInput(m_pInterferencePipeline->getOutputBuffer());
}

void App::createGUI() {
    LOG("App::createGUI");
    Window*     pWindow      = m_pWindow;
    Renderpass* pRenderpass  = m_pScreenSpacePipeline->getRenderpass();
    Image* heightMapImage    = m_pFluidPipeline->getHeightImage();
    Image* InterferenceImage = m_pInterferencePipeline->getOutputImage();
    
    m_pGUI = new GUI();
    m_pGUI->initGUI(pWindow, pRenderpass);
    m_pGUI->addHeightMapImage(heightMapImage);
    m_pGUI->addInterferenceImage(InterferenceImage);
    m_cleaner.push([=](){ m_pGUI->cleanupGUI(); });
}

void App::setup() {
    m_pCamera = new Camera();
    initWindow();
    initDevice();
    initCommander();
    createScreenSpacePipeline();
    createSwapchain();
    createMainPipeline();
    
    createFluidPipeline();
    createInterferencePipeline();
    dispatchInterference();
    
    createGUI();
}

void App::draw() {
    Swapchain* pSwapchain = m_pSwapchain;
    MainPipeline* pMainPipeline = m_pMainPipeline;
    FluidPipeline* pFluidPipeline = m_pFluidPipeline;
    ScreenSpacePipeline* pScreenSpacePipeline = m_pScreenSpacePipeline;
    GUI* pGUI = m_pGUI;
    
    pSwapchain->prepareFrame();
    Frame*      pCurrentFrame = pSwapchain->getCurrentFrame();
    VkCommandBuffer cmdBuffer = pSwapchain->getCommandBuffer();
    
    VkCommandBufferBeginInfo commandBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkResult result = vkBeginCommandBuffer(cmdBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    
    pFluidPipeline->dispatch(cmdBuffer);
    
    pMainPipeline->render(cmdBuffer);
    
    pScreenSpacePipeline->setFrame(pCurrentFrame);
    pScreenSpacePipeline->render(cmdBuffer, pGUI);
    
    vkEndCommandBuffer(cmdBuffer);
    
    pSwapchain->submitFrame();
    pSwapchain->presentFrame();
}

void App::update() {
    Settings* settings = System::Settings();
    if (settings->LockFocus) moveViewLock(m_pWindow);
    else                     moveView(m_pWindow);
    
    m_pMainPipeline->updateLightInput();
    m_pMainPipeline->updateCameraInput(m_pCamera);
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
    m_pMainPipeline->recreateFrame(size);
    m_pScreenSpacePipeline->setupInput(m_pMainPipeline->getFrame());
}

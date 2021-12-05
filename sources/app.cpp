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
    m_pWindow = new Window();
    m_pWindow->create({800, 600}, "Vulkan");
    m_pWindow->setWindowPosition(0, 0);
    m_pWindow->enableInput();
    m_cleaner.push([=](){ m_pWindow->close(); });
}

void App::initDevice() {
    m_pDevice = new Device();
    m_pDevice->setup();
    m_pDevice->createInstance();
    m_pDevice->createDebugMessenger();
    m_pDevice->createSurface(m_pWindow->getGLFWwindow());
    m_pDevice->selectPhysicalDevice();
    m_pDevice->createLogicalDevice();
    System::Instance().setDevice(m_pDevice);
    m_cleaner.push([=](){ m_pDevice->cleanup(); });
}

void App::initCommander() {
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
    m_pScreenSpacePipeline->createGUI(m_pWindow);
    m_cleaner.push([=](){ m_pScreenSpacePipeline->cleanup(); });
}

void App::createSwapchain() {
    m_pSwapchain = new Swapchain();
    m_pSwapchain->setup();
    m_pSwapchain->create();
    m_pSwapchain->createFrames(m_pScreenSpacePipeline->getRenderpass());
    m_cleaner.push([=](){ m_pSwapchain->cleanup(); });
}

void App::createInterferencePipeline() {
    LOG("App::renderInterference");
    m_pInterferencePipeline = new InterferencePipeline();
    m_pInterferencePipeline->setupShader();
    m_pInterferencePipeline->createDescriptor();
    m_pInterferencePipeline->setupInput(m_opdSample, m_refractiveIndex);
    m_pInterferencePipeline->setupOutput();
    m_pInterferencePipeline->createPipelineLayout();
    m_pInterferencePipeline->createPipeline();
    m_cleaner.push([=](){ m_pInterferencePipeline->cleanup(); });
    m_cleaner.push([=](){ m_pInterferencePipeline->getOutputBuffer()->cleanup(); });
}

void App::dispatchInterference() {
    VkCommandBuffer cmdBuffer = m_pCommander->createCommandBuffer();
    m_pCommander->beginSingleTimeCommands(cmdBuffer);
    m_pInterferencePipeline->dispatch(cmdBuffer);
    m_pCommander->endSingleTimeCommands(cmdBuffer);
}

void App::createMainPipeline() {
    LOG("App::createMainPipeline");
    UInt2D size = m_pWindow->getFrameSize();
    
    m_pMainPipeline = new MainPipeline();
    m_pMainPipeline->setupShader();
    m_pMainPipeline->createDescriptor();
    m_pMainPipeline->setupInput(m_opdSample);
    m_pMainPipeline->createRenderpass();
    m_pMainPipeline->createPipelineLayout();
    m_pMainPipeline->createPipeline();
    m_pMainPipeline->createFrame(size);
    m_cleaner.push([=](){ m_pMainPipeline->cleanup(); });
    
    m_pMainPipeline->updateInterferenceInput(m_pInterferencePipeline->getOutputBuffer());
    m_pScreenSpacePipeline->setupInput(m_pMainPipeline->getFrame()->getColorImage());
}

void App::setup() {
    m_pCamera = new Camera();
    initWindow();
    initDevice();
    initCommander();
    createScreenSpacePipeline();
    createSwapchain();
    
    createInterferencePipeline();
    dispatchInterference();
    
    createMainPipeline();
}

void App::loop() {
    LOG("App::loop");

    long iteration = 0;
    while (m_pWindow->isOpen()) {
        iteration++;
        m_pWindow->pollEvents();
        checkResized();
        update(iteration);
        draw(iteration);
        m_pWindow->resetInput();
    }
    m_pDevice->waitIdle();
}

void App::update(long iteration) {
    Settings* settings = System::Settings();
    if (settings->LockFocus) moveViewLock(m_pWindow);
    else                     moveView(m_pWindow);
    
    m_pMainPipeline->updateLightInput(iteration);
    m_pMainPipeline->updateCameraInput(m_pCamera);
    System::Settings()->CameraPos = m_pCamera->getPosition();
}

void App::draw(long iteration) {
    Swapchain* pSwapchain = m_pSwapchain;
    MainPipeline* pMainPipeline = m_pMainPipeline;
    ScreenSpacePipeline* pScreenSpacePipeline = m_pScreenSpacePipeline;
    
    pSwapchain->prepareFrame();
    Frame*      pCurrentFrame = pSwapchain->getCurrentFrame();
    VkCommandBuffer cmdBuffer = pSwapchain->getCommandBuffer();
    
    VkCommandBufferBeginInfo commandBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkResult result = vkBeginCommandBuffer(cmdBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    
    pMainPipeline->render(cmdBuffer);
    
    pScreenSpacePipeline->setFrame(pCurrentFrame);
    pScreenSpacePipeline->render(cmdBuffer);
    
    vkEndCommandBuffer(cmdBuffer);
    
    pSwapchain->submitFrame();
    pSwapchain->presentFrame();
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
    float scale = m_pCamera->getDistance() / 5.;
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += (pWindow->getKeyState(key_d) - pWindow->getKeyState(key_a)) * scale;
    movement.y += (pWindow->getKeyState(key_e) - pWindow->getKeyState(key_q)) * scale;
    movement.z += (pWindow->getKeyState(key_w) - pWindow->getKeyState(key_s)) * scale * 0.8;
    
    if (pWindow->getMouseBtnState(mouse_btn_left)) {
        glm::vec2 cursorOffset = pWindow->getCursorOffset();
        movement.x += cursorOffset.x * scale;
        movement.y += cursorOffset.y * scale;
    }
    movement.z += pWindow->getScrollOffset().y * scale * 0.8;
    m_pCamera->move(movement);
}

void App::checkResized() {
    if (!m_pWindow->checkResized()) return;
    LOG("App::resized");
    UInt2D size = m_pWindow->getFrameSize();
    
    m_pSwapchain->recreate();
    m_pMainPipeline->recreateFrame(size);
    m_pScreenSpacePipeline->setupInput(m_pMainPipeline->getFrame()->getColorImage());
}

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

void App::cleanup() {
    m_cleaner.flush();
}

void App::initWindow() {
    m_pWindow = new Window();
    m_pWindow->create({600, 600}, "Vulkan");
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

void App::createFramePipeline() {
    m_pFramePipeline = new FramePipeline();
    m_pFramePipeline->setupShader();
    m_pFramePipeline->createRenderpass();
    m_pFramePipeline->createPipelineLayout();
    m_pFramePipeline->createPipeline();
    m_cleaner.push([=](){ m_pFramePipeline->cleanup(); });
}

void App::createSwapchain() {
    m_pSwapchain = new Swapchain();
    m_pSwapchain->setup();
    m_pSwapchain->create();
    m_pSwapchain->createFrames(m_pFramePipeline->getRenderpass());
    m_cleaner.push([=](){ m_pSwapchain->cleanup(); });
}

void App::setup() {
    initWindow();
    initDevice();
    initCommander();
    createFramePipeline();
    createSwapchain();
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
    }
    m_pDevice->waitIdle();
}

void App::update(long iteration) {
    
}

void App::draw(long iteration) {
    Swapchain* pSwapchain = m_pSwapchain;
    FramePipeline* pFramePipeline = m_pFramePipeline;
    
    pSwapchain->prepareFrame();
    Frame*      pCurrentFrame = pSwapchain->getCurrentFrame();
    VkCommandBuffer cmdBuffer = pSwapchain->getCommandBuffer();
    
    VkCommandBufferBeginInfo commandBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkResult result = vkBeginCommandBuffer(cmdBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    
    pFramePipeline->setFrame(pCurrentFrame);
    pFramePipeline->render(cmdBuffer);
    
    vkEndCommandBuffer(cmdBuffer);
    
    pSwapchain->submitFrame();
    pSwapchain->presentFrame();
}

void App::checkResized() {
    if (!m_pWindow->checkResized()) return;
    LOG("App::resized");
    
    m_pSwapchain->recreate();
}

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

void App::createInterferencePipeline() {
    m_pInterferencePipeline = new InterferencePipeline();
    m_pInterferencePipeline->setupShader();
    m_pInterferencePipeline->setupInput();
    m_pInterferencePipeline->setupOutput();
    m_pInterferencePipeline->createDescriptor();
    m_pInterferencePipeline->createPipelineLayout();
    m_pInterferencePipeline->createPipeline();
    m_cleaner.push([=](){ m_pInterferencePipeline->cleanup(); });
}

void App::createScreenSpacePipeline() {
    m_pScreenSpacePipeline = new ScreenSpacePipeline();
    m_pScreenSpacePipeline->setupShader();
    m_pScreenSpacePipeline->createRenderpass();
    m_pScreenSpacePipeline->createPipelineLayout();
    m_pScreenSpacePipeline->createPipeline();
    m_cleaner.push([=](){ m_pScreenSpacePipeline->cleanup(); });
}

void App::createSwapchain() {
    m_pSwapchain = new Swapchain();
    m_pSwapchain->setup();
    m_pSwapchain->create();
    m_pSwapchain->createFrames(m_pScreenSpacePipeline->getRenderpass());
    m_cleaner.push([=](){ m_pSwapchain->cleanup(); });
}

void App::preRender() {
    Commander* pCommander = m_pCommander;
    VkCommandBuffer cmdBuffer = pCommander->createCommandBuffer();
    m_pCommander->beginSingleTimeCommands(cmdBuffer);
    m_pInterferencePipeline->dispatch(cmdBuffer);
    m_pCommander->endSingleTimeCommands(cmdBuffer);
}

void App::setup() {
    initWindow();
    initDevice();
    initCommander();
    createScreenSpacePipeline();
    createSwapchain();
    
    createInterferencePipeline();
    preRender();
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
    ScreenSpacePipeline* pScreenSpacePipeline = m_pScreenSpacePipeline;
    
    pSwapchain->prepareFrame();
    Frame*      pCurrentFrame = pSwapchain->getCurrentFrame();
    VkCommandBuffer cmdBuffer = pSwapchain->getCommandBuffer();
    
    VkCommandBufferBeginInfo commandBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkResult result = vkBeginCommandBuffer(cmdBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    
    pScreenSpacePipeline->setFrame(pCurrentFrame);
    pScreenSpacePipeline->render(cmdBuffer);
    
    vkEndCommandBuffer(cmdBuffer);
    
    pSwapchain->submitFrame();
    pSwapchain->presentFrame();
}

void App::checkResized() {
    if (!m_pWindow->checkResized()) return;
    LOG("App::resized");
    
    m_pSwapchain->recreate();
}

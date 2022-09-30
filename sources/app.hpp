//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window/window.hpp"
#include "window/gui.hpp"
#include "renderer/device.hpp"
#include "renderer/commander.hpp"
#include "renderer/swapchain.hpp"
#include "pipelines/graphics_screen.hpp"
#include "pipelines/compute_hdr.hpp"
#include "pipelines/compute_brdf.hpp"
#include "pipelines/compute_interference.hpp"
#include "pipelines/compute_fluid.hpp"
#include "pipelines/compute_marking.hpp"
#include "pipelines/compute_rain.hpp"
#include "pipelines/graphics_reflection.hpp"
#include "pipelines/graphics_scene.hpp"
#include "pipelines/graphics_equirect.hpp"
#include "resources/camera.hpp"
#include "resources/buffer.hpp"

class App {
public:
    
    void run();

private:
    Cleaner m_cleaner;
    Window* m_pWindow;
    Device* m_pDevice;
    Commander* m_pCommander;
    
    Camera* m_pCamera;
    GUI*    m_pGUI;
    
    Swapchain* m_pSwapchain;
    GraphicsScreen* m_pGraphicsScreen;
    GraphicsScene* m_pGraphicsScene;
    
    ComputeFluid* m_pComputeFluid;
    ComputeMarking* m_pComputeMarking;
    ComputeRain* m_pComputeRain;
    
    void cleanup();
    void setup();
    void loop();
    void update();
    void draw();
    
    void initWindow();
    void initDevice();
    void initCommander();
    
    void createSwapchain();
    void createGraphicsScreen();
    void createInterference();
    void createComputeFluid();
    void createComputeMarking();
    void createComputeRain();
    void dispatchInterference();
    void createGraphicsScene();
    
    void createCubemap();
    
    void createGUI();
    
    void moveView(Window* pWindow);
    void moveViewLock(Window* pWindow);
    
    void checkResized();
    
};


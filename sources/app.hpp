//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window/window.hpp"
#include "window/gui.hpp"
#include "renderer/device.hpp"
#include "renderer/commander.hpp"
#include "renderer/swapchain.hpp"
#include "pipelines/screenspace_pipeline.hpp"
#include "pipelines/interference_pipeline.hpp"
#include "pipelines/fluid_pipeline.hpp"
#include "pipelines/main_pipeline.hpp"
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
    ScreenSpacePipeline* m_pScreenSpacePipeline;
    InterferencePipeline* m_pInterferencePipeline;
    MainPipeline* m_pMainPipeline;
    
    FluidPipeline* m_pFluidPipeline;
    
    void cleanup();
    void setup();
    void loop();
    void update();
    void draw();
    
    void initWindow();
    void initDevice();
    void initCommander();
    
    void createSwapchain();
    void createScreenSpacePipeline();
    void createInterferencePipeline();
    void createFluidPipeline();
    void dispatchInterference();
    void createMainPipeline();
    
    void createGUI();
    
    void moveView(Window* pWindow);
    void moveViewLock(Window* pWindow);
    
    void checkResized();
    
};


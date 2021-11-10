//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window/window.hpp"
#include "renderer/device.hpp"
#include "renderer/commander.hpp"
#include "renderer/swapchain.hpp"
#include "pipelines/frame_pipeline.hpp"

class App {
public:
    
    void run();

private:
    Cleaner m_cleaner;
    Window* m_pWindow;
    Device* m_pDevice;
    Commander* m_pCommander;
    
    Swapchain* m_pSwapchain;
    FramePipeline* m_pFramePipeline;
    
    
    void cleanup();
    void setup();
    void loop();
    void update(long iteration);
    void draw(long iteration);
    
    void initWindow();
    void initDevice();
    void initCommander();
    
    void createSwapchain();
    void createFramePipeline();
    
    void checkResized();
    
};


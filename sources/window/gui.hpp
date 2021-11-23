//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "window.hpp"

#include "../extensions/ext_imgui.h"

class GUI {
public:
    
    GUI();
    ~GUI();
    
    void cleanupGUI();
    
    void setWindow(Window* window);

    void initGUI(VkRenderPass renderPass);
    void renderGUI(VkCommandBuffer commandBuffer);
    
private:
    
    Cleaner m_cleaner;
    Window* m_pWindow;
    
    void drawStatusWindow();
};


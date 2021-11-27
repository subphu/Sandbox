//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "window.hpp"
#include "../renderer/renderpass.hpp"

#include "../extensions/ext_imgui.h"

class GUI {
public:
    
    GUI();
    ~GUI();
    
    void cleanupGUI();

    void initGUI(Window* pWindow, Renderpass* pRenderpass);
    void renderGUI(VkCommandBuffer commandBuffer);
    
private:
    Cleaner m_cleaner;
    Window* m_pWindow;
    
    ImGui_ImplVulkan_InitInfo m_initInfo{};
    
    void drawStatusWindow();
};


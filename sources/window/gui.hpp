//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "window.hpp"
#include "../renderer/renderpass.hpp"
#include "image.hpp"

#include "../extensions/ext_imgui.h"

class GUI {
public:
    
    GUI();
    ~GUI();
    
    void cleanupGUI();

    void initGUI(Window* pWindow, Renderpass* pRenderpass);
    void renderGUI(VkCommandBuffer cmdBuffer);
    
    void addImage(Image* image);
    
private:
    Cleaner m_cleaner;
    Window* m_pWindow;
    ImTextureID m_imTexture;
    
    ImGui_ImplVulkan_InitInfo m_initInfo{};
    
    void drawStatusWindow();
};


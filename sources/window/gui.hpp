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
    
    void addInterferenceImage(Image* pImage);
    void updateHeightMapImage(Image* pImage);
    void updateIridescentImage(Image* pImage);
    void updateFluidImage(Image* pImage);
    
    void addCubemapImage(Image* pImage);
    void addTextureImage(Image* pImage);
    
private:
    Cleaner m_cleaner;
    Window* m_pWindow;
    
    ImTextureID m_interferenceTexID;
    ImTextureID m_fluidTexID;
    ImTextureID m_heightMapTexID;
    ImTextureID m_iridescentTexID;
    
    ImTextureID m_textureTexID;
    ImTextureID m_cubemapTexID;
    
    ImGui_ImplVulkan_InitInfo m_initInfo{};
    
    void changeStyle();
    void drawStatusWindow();
    void drawImageWindow();
};


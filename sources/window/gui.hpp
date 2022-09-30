//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"
#include "window.hpp"
#include "../renderer/renderpass.hpp"
#include "../resources/image.hpp"

#include "../extensions/ext_imgui.h"

class GUI {
public:
    
    GUI();
    ~GUI();
    
    void cleanupGUI();

    void initGUI(Window* pWindow, Renderpass* pRenderpass);
    void renderGUI(VkCommandBuffer cmdBuffer);
    
    void addInterferenceImage(Image* pImage);
    void addMarkedImage(Image* pImage);
    void updateHeightMapImage(Image* pImage);
    void updateIridescentImage(Image* pImage);
    void updateFluidImage(Image* pImage);
    
    void addCubemapImage(Image* pImage);
    void addTextureImage(Image* pImage);
    
    void addCubemapPrev(VECTOR<Image*> pImages);
    void addTexturePrev(VECTOR<Image*> pImages);
    
private:
    Cleaner m_cleaner;
    Window* m_pWindow;
    
    ImTextureID m_interferenceTexID;
    ImTextureID m_markedTexID;
    ImTextureID m_fluidTexID;
    ImTextureID m_heightMapTexID;
    ImTextureID m_iridescentTexID;
    
    VECTOR<ImTextureID> m_cubemapPrevID;
    VECTOR<ImTextureID> m_texturePrevID;
    
    ImTextureID m_textureTexID;
    ImTextureID m_cubemapTexID;
    
    ImGui_ImplVulkan_InitInfo m_initInfo{};
    
    void changeStyle();
    void drawStatusWindow();
    void drawImageWindow();
    void drawTransparentWindow();
};


//  Copyright © 2021 Subph. All rights reserved.
//

#include "gui.hpp"

#include "../system.hpp"

GUI::~GUI() { }
GUI::GUI() { }

void GUI::cleanupGUI() { m_cleaner.flush("GUI"); }

void GUI::initGUI(Window* pWindow, Renderpass* pRenderpass) {
    LOG("Settings::initGUI");
    Device*      pDevice     = System::Device();
    VkDevice     device      = pDevice->getDevice();
    VkRenderPass renderpass  = pRenderpass->get();
    GLFWwindow*  pGlfwWindow = pWindow->getGLFWwindow();
    
    m_initInfo.Instance       = pDevice->getInstance();
    m_initInfo.PhysicalDevice = pDevice->getPhysicalDevice();
    m_initInfo.Device         = pDevice->getDevice();
    m_initInfo.Queue          = pDevice->getGraphicQueue();
    m_initInfo.DescriptorPool = IMGUI::CreateDescPool(device);
    m_initInfo.MinImageCount  = 3;
    m_initInfo.ImageCount     = 3;
    m_initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    
    ImGui::CreateContext();
    
    ImGui_ImplGlfw_InitForVulkan(pGlfwWindow, false);
    ImGui_ImplVulkan_Init(&m_initInfo, renderpass);
    
    IMGUI::CreateFontsTexture(System::Commander());
    
    m_cleaner.push([=]() {
        vkDestroyDescriptorPool(device, m_initInfo.DescriptorPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
    });
    m_pWindow = pWindow;
    
    Files* pFiles = System::Files();
    addCubemapPrev(pFiles->getCubemapPreviews());
    addTexturePrev(pFiles->getTexturePreviews());
}

void GUI::renderGUI(VkCommandBuffer cmdBuffer) {
    Settings* settings = System::Settings();
    
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    changeStyle();
    if (settings->ShowDemo) ImGui::ShowDemoWindow(&settings->ShowDemo);
    drawStatusWindow();
    drawImageWindow();
    drawTransparentWindow();
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
}

void GUI::drawTransparentWindow() {
    Settings* settings = System::Settings();
    ImGuiStyle& style = ImGui::GetStyle();
    UInt2D  windowSize = m_pWindow->getSize();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.,0.,0.,0.);
    style.WindowBorderSize = 0;

    ImVec2 windowPos = ImVec2(windowSize.width/2 - 300,
                              windowSize.height - 120);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(600, 120), ImGuiCond_Once);
    ImGui::Begin("Label Image", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::Image(m_markedTexID, {600, 50});
    ImGui::Image(m_interferenceTexID, {600, 50});
    ImGui::End();
    
    windowPos = ImVec2(windowSize.width/2 - 300, 24);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.,0.,0.,0.);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(600, 120), ImGuiCond_Once);
    ImGui::Begin("Label Text", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowFontScale(1.2);
    ImGui::Image(m_heightMapTexID, {100, 100});
    ImGui::SameLine();
    ImGui::Image(m_iridescentTexID, {100, 100});
    
    ImGui::End();
}

void GUI::drawStatusWindow() {
    Settings* settings = System::Settings();
    UInt2D  windowSize = m_pWindow->getSize();
    
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, windowSize.height), ImGuiCond_Once);
    ImGui::Begin("Status");
    
    ImGui::Checkbox("Lock", &settings->LockFPS);
    ImGui::SameLine();
    ImGui::Text("FPS %.1f (%.3f ms/fr)",
                ImGui::GetIO().Framerate,
                1000.0f / ImGui::GetIO().Framerate);
    
    ImGui::Checkbox("Focus,", &settings->LockFocus);
    ImGui::SameLine();
    ImGui::Text("x:%.2f y:%.2f z:%.2f",
                settings->CameraPos.x, settings->CameraPos.y, settings->CameraPos.z);
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::SliderInt("Total", &settings->TotalLight, 0, 4);
        ImGui::Checkbox("Move Light", &settings->LightMove);
        ImGui::DragFloat2("Distance", (float*) &settings->Distance, 0.05f);
        ImGui::DragFloat("Radiance", &settings->Radiance, 10.f, 0.f, 10000.f);
        ImGui::ColorEdit3("Color", (float*) &settings->LightColor);
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Interferences")) {
        ImGui::Checkbox("Interference", &settings->Interference);
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
        ImGui::SliderFloat("Thickness", &settings->ThicknessScale, 0.f, 2.f);
        ImGui::SliderFloat("Refractive", &settings->RefractiveIndex, 1.f, 4.f);
        ImGui::SliderFloat("OPD Offset", &settings->OPDOffset, -.5f, .5f);
        ImGui::SliderFloat("Reflectance", &settings->ReflectanceValue, 0.f, 1.f);
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Materials")) {
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
        ImGui::Checkbox("Move Object", &settings->ObjectMove);
        ImGui::SliderInt("Shapes", &settings->Shapes, 0, 2);
        ImGui::ColorEdit4("Albedo", (float*) &settings->Albedo);
        ImGui::SliderFloat("Metallic", &settings->Metallic, 0.f, 1.f);
        ImGui::SliderFloat("Roughness", &settings->Roughness, 0.f, 1.f);
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Template")) {
        if (ImGui::Button("Bubble")) {
            LOG("Button::Template Bubble");
            settings->UseHeightmap   = true;
            settings->TotalLight = 0;
            settings->Albedo     = {1.f, 1.f, 1.f, .1f};
            settings->ThicknessScale  = 0.45;
            settings->RefractiveIndex = 1.5;
            settings->OPDOffset = 0.05;
            settings->Roughness = 0.;
            settings->Shapes    = 0;
        }
        if (ImGui::Button("Steel")) {
            LOG("Button::Template Steel");
            settings->UseHeightmap   = false;
            settings->TotalLight = 2;
            settings->Radiance   = 250.f;
            settings->Albedo     = {1.f, 1.f, 1.f, 1.f};
            settings->ThicknessScale  = 0.1;
            settings->RefractiveIndex = 3.;
            settings->OPDOffset = 0.;
            settings->Roughness = .5;
            settings->Shapes    = 1;
        }
        if (ImGui::Button("Metal")) {
            LOG("Button::Template Steel");
            settings->UseHeightmap = false;
            settings->TotalLight   = 2;
            settings->Radiance     = 200.f;
            settings->Albedo       = {1.f, 1.f, 1.f, 1.f};
            settings->ThicknessScale  = 0.42;
            settings->RefractiveIndex = 1.35;
            settings->OPDOffset   = 0.;
            settings->Roughness   = 0.;
            settings->Shapes      = 2;
        }
    }
    
    ImGui::Separator();
    ImGui::Checkbox("Show ImGUI demo", &settings->ShowDemo);
    
    ImGui::End();
}

void GUI::drawImageWindow() {
    Settings* settings = System::Settings();
    Files* pFiles = System::Files();
    UInt2D windowSize = m_pWindow->getSize();
    ImVec2 windowPos = ImVec2(windowSize.width - 250, 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, windowSize.height), ImGuiCond_Once);
    ImGui::Begin("Images");
    ImGui::Checkbox("Use Heightmap", &settings->UseHeightmap);
    ImGui::Checkbox("Simulate Fluid", &settings->RunFluid);
    ImGui::Checkbox("Simulate Rain", &settings->RunRain);
    if (ImGui::BeginTabBar("FluidTabBar")) {
        if (ImGui::BeginTabItem("Height")) {
            ImGui::Image(m_heightMapTexID, {234, 234});
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fluid")) {
            ImGui::Image(m_fluidTexID, {234, 234});
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Iridesence")) {
            ImGui::Image(m_iridescentTexID, {234, 234});
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Interference Image")) {
        ImGui::Image(m_interferenceTexID, {234, 30});
        ImGui::Image(m_markedTexID, {234, 30});
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::BeginTabBar("Texture Files")) {
        if (ImGui::BeginTabItem("Cubemap")) {
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
            ImGui::SliderInt("Cubemaps", &settings->Cubemaps, 0, pFiles->getTotalCubemap()-1);
            ImGui::Image(m_cubemapPrevID[settings->Cubemaps], {234, 117});
            if (ImGui::Button("Update Cubemap")) {
                LOG("Button::Cubemap Update");
                pFiles->setCubemapIdx(settings->Cubemaps);
                settings->BtnUpdateCubemap = true;
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Textures")) {
            ImGui::Checkbox("##UseTexture", &settings->UseTexture);
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.55f);
            ImGui::SameLine();
            ImGui::SliderInt("Textures", &settings->Textures, 0, pFiles->getTotalTexture()-1);
            ImGui::Image(m_texturePrevID[settings->Textures], {234, 234});
            if (ImGui::Button("Update Texture")) {
                LOG("Button::Texture Update");
                pFiles->setTextureIdx(settings->Textures);
                settings->BtnUpdateTexture = true;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void GUI::changeStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.,0.,0.,0.5);
}

void GUI::addInterferenceImage(Image* pImage) {
    m_interferenceTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::addMarkedImage(Image* pImage) {
    m_markedTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::updateHeightMapImage(Image* pImage) {
    pImage->cmdTransitionToShaderR();
    m_heightMapTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::updateIridescentImage(Image* pImage) {
    pImage->cmdTransitionToShaderR();
    m_iridescentTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::updateFluidImage(Image* pImage) {
    pImage->cmdTransitionToShaderR();
    m_fluidTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::addCubemapImage(Image* pImage) {
    m_cubemapTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::addTextureImage(Image* pImage) {
    m_textureTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::addCubemapPrev(VECTOR<Image*> pImages) {
    for (Image* pImage : pImages) {
        ImTextureID texID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
        m_cubemapPrevID.push_back(texID);
    }
}

void GUI::addTexturePrev(VECTOR<Image*> pImages) {
    for (Image* pImage : pImages) {
        ImTextureID texID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
        m_texturePrevID.push_back(texID);
    }
}

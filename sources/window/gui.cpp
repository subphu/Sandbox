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
    
    ImGui_ImplGlfw_InitForVulkan(pGlfwWindow, nullptr);
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
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
}

void GUI::drawStatusWindow() {
    Settings* settings = System::Settings();
    UInt2D  windowSize = m_pWindow->getSize();
    
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, windowSize.height));
    ImGui::Begin("Status");
    
    ImGui::Checkbox("Lock", &settings->LockFPS);
    ImGui::SameLine();
    ImGui::Text("FPS %.1f (%.3f ms/fr)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    
    ImGui::Checkbox("Focus,", &settings->LockFocus);
    ImGui::SameLine();
    ImGui::Text("x:%.2f y:%.2f z:%.2f",
                settings->CameraPos.x, settings->CameraPos.y, settings->CameraPos.z);
    
//    ImGui::ColorEdit3("Clear", (float*) &settings->ClearColor);
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::SliderInt("Total", &settings->TotalLight, 0, 4);
        ImGui::DragFloat2("Distance", (float*) &settings->Distance, 0.05f);
        ImGui::DragFloat("Radiance", &settings->Radiance, 10.f, 0.f, 10000.f);
        ImGui::ColorEdit3("Color", (float*) &settings->LightColor);
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Interferences")) {
        ImGui::Checkbox("Interference", &settings->Interference);
        ImGui::SameLine();
        ImGui::Checkbox("Phase Shift", &settings->PhaseShift);
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
        ImGui::SliderFloat("Thickness", &settings->ThicknessScale, 0.f, 1.f);
        ImGui::SliderFloat("Refractive", &settings->RefractiveIndex, 0.f, 4.f);
        ImGui::SliderFloat("Reflectance", &settings->ReflectanceValue, 0.f, 1.f);
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Materials")) {
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
        ImGui::SliderInt("Shapes", &settings->Shapes, 1, 4);
        ImGui::ColorEdit4("Albedo", (float*) &settings->Albedo);
        ImGui::SliderFloat("Metallic", &settings->Metallic, 0.f, 1.f);
        ImGui::SliderFloat("Roughness", &settings->Roughness, 0.f, 1.f);
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
    ImGui::SetNextWindowSize(ImVec2(250, windowSize.height));
    ImGui::Begin("Images");
    ImGui::Checkbox("Use Fluid", &settings->UseFluid);
    ImGui::Checkbox("Simulate Fluid", &settings->RunFluid);
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
        ImGui::Image(m_interferenceTexID, {234, 65});
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::BeginTabBar("Texture Files")) {
        if (ImGui::BeginTabItem("Textures")) {
            ImGui::Checkbox("##UseTexture", &settings->UseTexture);
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.55f);
            ImGui::SameLine();
            ImGui::SliderInt("Textures", &settings->Textures, 0, pFiles->getTotalTexture()-1);
            ImGui::Image(m_texturePrevID[settings->Textures], {234, 234});
            if (ImGui::Button("Update Texture")) {
                LOG("Button::Texture Update");
                pFiles->setTextureIdx(settings->Textures);
                settings->btnUpdateTexture = true;
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cubemap")) {
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
            ImGui::SliderInt("Cubemaps", &settings->Cubemaps, 0, pFiles->getTotalCubemap()-1);
            ImGui::Image(m_cubemapPrevID[settings->Cubemaps], {234, 117});
            if (ImGui::Button("Update Cubemap")) {
                LOG("Button::Cubemap Update");
                pFiles->setCubemapIdx(settings->Cubemaps);
                settings->btnUpdateCubemap = true;
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

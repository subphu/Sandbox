//  Copyright © 2021 Subph. All rights reserved.
//

#include "gui.hpp"

#include "../system.hpp"

GUI::~GUI() { }
GUI::GUI() { }

void GUI::cleanupGUI() { m_cleaner.flush("Settings"); }

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
}

void GUI::renderGUI(VkCommandBuffer cmdBuffer) {
    Settings* settings = System::Settings();
    
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    changeStyle();
    if (settings->ShowDemo) ImGui::ShowDemoWindow(&settings->ShowDemo);
    drawStatusWindow();
    drawInterferenceWindow();
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
}

void GUI::drawStatusWindow() {
    Settings* settings = System::Settings();
    UInt2D  windowSize = m_pWindow->getSize();
    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
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
    
    ImGui::ColorEdit3("Clear", (float*) &settings->ClearColor);
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true);
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::SliderInt("Total", &settings->TotalLight, 1, 4);
        ImGui::DragFloat2("Distance", (float*) &settings->Distance, 0.05f);
        ImGui::DragFloat("Radiance", &settings->Radiance, 10.f, 0.f, 10000.f);
        ImGui::ColorEdit3("Color", (float*) &settings->LightColor);
    }
    ImGui::Separator();
    
    ImGui::Checkbox("Show ImGUI demo", &settings->ShowDemo);
    ImGui::Image(m_heightMapTexID, {234, 234});
    ImGui::Checkbox("Simulate Fluid", &settings->RunFluid);
    
    ImGui::End();
}

void GUI::drawInterferenceWindow() {
    ImGui::SetNextWindowPos(ImVec2(250, 0));
    ImGui::SetNextWindowSize(ImVec2(420, 75));
    ImGui::Begin("Interference");
    ImGui::Image(m_interferenceTexID, {400, 40});
    ImGui::End();
}

void GUI::changeStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.,0.,0.,0.2);
}

void GUI::addInterferenceImage(Image* pImage) {
    pImage->cmdTransitionToShaderR();
    m_interferenceTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

void GUI::addHeightMapImage(Image* pImage) {
    pImage->cmdTransitionToShaderR();
    m_heightMapTexID = (ImTextureID)ImGui_ImplVulkan_CreateTexture(pImage->getSampler(), pImage->getImageView(), pImage->getImageLayout());
}

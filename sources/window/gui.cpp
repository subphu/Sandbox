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
}

void GUI::renderGUI(VkCommandBuffer cmdBuffer) {
    Settings* settings = System::Settings();
    
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    if (settings->ShowDemo) ImGui::ShowDemoWindow(&settings->ShowDemo);
    drawStatusWindow();
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
}

void GUI::drawStatusWindow() {
    Settings* settings = System::Settings();
    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
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
    
    
    ImGui::ColorEdit3("Clear Color", (float*) &settings->ClearColor);
    
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::SliderInt("Total", &settings->TotalLight, 1, 4);
        ImGui::DragFloat2("Distance", (float*) &settings->Distance, 0.05f);
        ImGui::DragFloat("Radiance", &settings->Radiance, 10.f, 0.f, 10000.f);
        ImGui::ColorEdit3("Color", (float*) &settings->LightColor);
    }
    ImGui::Separator();
    
    
    ImGui::Checkbox("Show ImGUI demo", &settings->ShowDemo);
    
//    ImGui::Image(m_imTexture, {100, 100});
//    if (ImGui::Button("Button"))
//        counter++;
//    ImGui::SameLine();
//    ImGui::Text("counter = %d", counter);
    
    
    ImGui::End();
}

void GUI::addImage(Image* image) {
    m_imTexture = (ImTextureID)ImGui_ImplVulkan_CreateTexture(image->getSampler(), image->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

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

void GUI::renderGUI(VkCommandBuffer commandBuffer) {
    Settings* settings = System::Settings();
    
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    if (settings->ShowDemo) ImGui::ShowDemoWindow(&settings->ShowDemo);
    drawStatusWindow();
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void GUI::drawStatusWindow() {
    Settings* settings = System::Settings();
    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Status");
    
    ImGui::Text("%.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    
    ImGui::Text("Camera x:%.2f y:%.2f z:%.2f",
                settings->CameraPos.x, settings->CameraPos.y, settings->CameraPos.z);
    
    ImGui::Checkbox("Lock FPS"  , &settings->LockFPS);
    ImGui::Checkbox("Lock Focus", &settings->LockFocus);
    
    ImGui::ColorEdit3("Clear Color", (float*) &settings->ClearColor);

//    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
//    if (ImGui::Button("Button"))
//        counter++;
//    ImGui::SameLine();
//    ImGui::Text("counter = %d", counter);
    
    ImGui::Checkbox("Show ImGUI demo", &settings->ShowDemo);
    
    ImGui::End();
}

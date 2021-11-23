//  Copyright © 2021 Subph. All rights reserved.
//

#include "gui.hpp"

#include "../system.hpp"

GUI::~GUI() { }
GUI::GUI() { }

void GUI::setWindow(Window* window) { m_pWindow = window; }


void GUI::cleanupGUI() { m_cleaner.flush("Settings"); }

void GUI::initGUI(VkRenderPass renderPass) {
    LOG("Settings::initGUI");
    GLFWwindow* pWindow = m_pWindow->getGLFWwindow();
    Device*     pDevice = System::Device();
    VkDevice    device  = pDevice->getDevice();
    
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance       = pDevice->getInstance();
    initInfo.PhysicalDevice = pDevice->getPhysicalDevice();
    initInfo.Device         = pDevice->getDevice();
    initInfo.Queue          = pDevice->getGraphicQueue();
    initInfo.DescriptorPool = IMGUI::CreateDescPool(device);
    initInfo.MinImageCount  = 3;
    initInfo.ImageCount     = 3;
    initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    
    ImGui::CreateContext();
    
    ImGui_ImplGlfw_InitForVulkan(pWindow, nullptr);
    ImGui_ImplVulkan_Init(&initInfo, renderPass);
    
    IMGUI::CreateFontsTexture(System::Commander());
    
    m_cleaner.push([=]() {
        vkDestroyDescriptorPool(device, initInfo.DescriptorPool, nullptr);
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

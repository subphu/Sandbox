//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../include.h"

enum key {
    key_esc = GLFW_KEY_ESCAPE,
    key_w = GLFW_KEY_W,
    key_a = GLFW_KEY_A,
    key_s = GLFW_KEY_S,
    key_d = GLFW_KEY_D,
    key_q = GLFW_KEY_Q,
    key_e = GLFW_KEY_E,
    key_z = GLFW_KEY_Z,
    key_x = GLFW_KEY_X,
    key_c = GLFW_KEY_C,
    key_r = GLFW_KEY_R,
    mouse_btn_left = 0,
    mouse_btn_right = 1,
};

class Window {
    
public:
    Window();
    ~Window();
    
    VkSurfaceKHR createSurface(VkInstance instance);
    VkSurfaceKHR getSurface();
    
    void create(UInt2D size, const char* name);
    void close();
    
    bool isOpen();
    float getRatio();
    UInt2D getSize();
    UInt2D getFrameSize();
    
    void setSize(UInt2D size);
    void notifyResize();
    bool checkResized();
    
    void enableInput();
    void pollEvents();
    void resetInput();
    bool getKeyState(int key);
    bool getMouseBtnState(int idx);
    
    glm::vec2 getCursorPosition();
    glm::vec2 getCursorOffset();
    glm::vec2 getScrollOffset();
    GLFWwindow* getGLFWwindow();
    
    void setCursorPosition(glm::vec2 pos);
    void setWindowPosition(uint x, uint y);
    void setMouseButton(int button, int action);
    void setScroll(double xoffset, double yoffset);
    
private:
    GLFWwindow* m_pWindow;
    const char* m_name    = "Vulkan";
    UInt2D      m_size    = { 900, 900 };
    bool        m_resized = false;
    
    glm::vec2 m_cursorPos;
    glm::vec2 m_cursorOffset;
    glm::vec2 m_scrollOffset;
    bool m_mouseBtn[3] = {0, 0, 0};
    
    static void ResizeCallback(GLFWwindow* window, int width, int height);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

};


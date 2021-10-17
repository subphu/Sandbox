//  Copyright © 2021 Subph. All rights reserved.
//

#include "window.hpp"

Window::Window() { }
Window::~Window() { }

void Window::create(Size<int> size, const char* name) {
    LOG("Window::create");
    m_name = name;
    setSize(size);
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    m_pWindow = glfwCreateWindow(size.width, size.height, name, nullptr, nullptr);
    CHECK_NULLPTR(m_pWindow, "Failed to create GLFW window");
    
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, ResizeCallback);
    
}

void Window::close() {
    LOG("Window::close");
    glfwSetWindowShouldClose(m_pWindow, true);
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

void Window::enableInput() {
    glfwSetInputMode(m_pWindow, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(m_pWindow, this->MouseButtonCallback);
    glfwSetScrollCallback(m_pWindow, ScrollCallback);
    resetInput();
}

void Window::resetInput() {
    double x, y;
    glfwGetCursorPos(m_pWindow, &x, &y);
    glm::vec2 cursorPos = glm::vec2(x, -y);
    m_cursorOffset      = m_cursorPos - cursorPos;
    m_cursorPos         = cursorPos;
    m_scrollOffset      = glm::vec2(0, 0);
}

void Window::pollEvents() {
    glfwPollEvents();
}

float Window::getRatio() {
    Size<int> size = getSize();
    return (float)size.width / (float)size.height;
}

Size<int> Window::getFrameSize () {
    int width, height;
    glfwGetFramebufferSize(m_pWindow, &width, &height);
    return { width, height };
}

Size<int> Window::getSize () {
    int width, height;
    glfwGetWindowSize(m_pWindow, &width, &height);
    return { width, height };
}

void Window::setSize(Size<int> size) {
    glfwSetWindowSize(m_pWindow, size.width, size.height);
}

void Window::setWindowPosition(uint x, uint y) {
    glfwSetWindowPos(m_pWindow, x, y);
}

void Window::setMouseButton(int button, int action) {
    m_mouseBtn[mouse_btn_left] = button == GLFW_MOUSE_BUTTON_LEFT ?
        action == GLFW_PRESS : m_mouseBtn[mouse_btn_left];
    m_mouseBtn[mouse_btn_right] = button == GLFW_MOUSE_BUTTON_LEFT ?
        action == GLFW_PRESS : m_mouseBtn[mouse_btn_right];
}

void Window::setScroll(double xoffset, double yoffset) {
    m_scrollOffset = glm::vec2(xoffset, yoffset);
}

void Window::notifyResize() {
    m_resized = true;
}

bool Window::checkResized() {
    bool result = m_resized;
    m_resized = false;
    return result;
}

bool  Window::isOpen() { return !glfwWindowShouldClose(m_pWindow); }
bool  Window::getKeyState     (int key) { return glfwGetKey(m_pWindow, key); }
bool  Window::getMouseBtnState(int idx) { return m_mouseBtn[idx]; }
glm::vec2 Window::getCursorPosition() { return m_cursorPos; }
glm::vec2 Window::getCursorOffset() { return m_cursorOffset; }
glm::vec2 Window::getScrollOffset() { return m_scrollOffset; }
GLFWwindow* Window::getGLFWwindow() { return m_pWindow; }

glm::vec2 Window::getCursorMovement() {
    if (!getMouseBtnState(mouse_btn_left)) {
        resetInput();
        return glm::vec2(0, 0);
    }
    glm::vec2 lastPos = m_cursorPos;
    if (lastPos.x == 0 && lastPos.y == 0) {
        resetInput();
        return glm::vec2(0, 0);
    }
    resetInput();
    glm::vec2 newPos = m_cursorPos;
    glm::vec2 move = glm::vec2(newPos.x - lastPos.x, newPos.y - lastPos.y);
    return glm::clamp(move, glm::vec2(-99, -99), glm::vec2(99, 99));
}

void Window::ResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->notifyResize();
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->setMouseButton(button, action);
}

void Window::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->setScroll(xoffset, yoffset);
}

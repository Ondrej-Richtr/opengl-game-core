#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


GLFWwindow *WindowManager::m_window = NULL;
glm::ivec2 WindowManager::m_win_size = glm::ivec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
glm::ivec2 WindowManager::m_framebuffer_size = glm::ivec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

void WindowManager::windowResizeCallback(GLFWwindow* window, int width, int height)
{
    //TODO check for resizes to 0x0?
    m_win_size = glm::ivec2(width, height);

    printf("Window resized to: %dx%d\n", m_win_size.x, m_win_size.y);
}

void WindowManager::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    //TODO check for resizes to 0x0?
    m_framebuffer_size = glm::ivec2(width, height);

    printf("Framebuffer resized to: %dx%d\n", m_framebuffer_size.x, m_framebuffer_size.y);
}

void WindowManager::init(GLFWwindow *window)
{
    assert(window != NULL);
    m_window = window;

    glfwSetWindowSizeCallback(m_window, windowResizeCallback);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    
    // calling this just to be sure with proper initialization
    windowResizeCallback(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    framebufferResizeCallback(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

GLFWwindow* WindowManager::getWindow()
{
    return m_window;
}

glm::ivec2 WindowManager::getSize()
{
    return m_win_size;
}

glm::vec2 WindowManager::getSizeF()
{
    return m_win_size;
}

glm::ivec2 WindowManager::getFBOSize()
{
    return m_framebuffer_size;
}

glm::vec2 WindowManager::getFBOSizeF()
{
    return m_framebuffer_size;
}

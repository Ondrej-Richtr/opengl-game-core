#include "game.hpp"


glm::vec2 MouseManager::mouse_pos(0.f);
bool MouseManager::left_button = false;
bool MouseManager::right_button = false;

void MouseManager::init(GLFWwindow *window)
{
    glfwSetCursorPosCallback(window, &MouseManager::mousePositionCallback);
    glfwSetMouseButtonCallback(window, &MouseManager::mouseButtonsCallback);
}

glm::ivec2 MouseManager::mousePos()
{
    return mouse_pos; // implicit conversion to integer vector
}

glm::ivec2 MouseManager::mousePosF()
{
    return mouse_pos;
}

void MouseManager::setCursorLocked()
{
    GLFWwindow* window = WindowManager::getWindow();
    if (window != NULL)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        fprintf(stderr, "[WARNING] Failed to lock cursor - window is NULL!");
    }
}

void MouseManager::setCursorVisible()
{
    GLFWwindow* window = WindowManager::getWindow();
    if (window != NULL)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        fprintf(stderr, "[WARNING] Failed to set cursor visible - window is NULL!");
    }
}

void MouseManager::mousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    mouse_pos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
}

void MouseManager::mouseButtonsCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        MouseManager::left_button = (action == GLFW_PRESS);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        MouseManager::right_button = (action == GLFW_PRESS);
    }
}

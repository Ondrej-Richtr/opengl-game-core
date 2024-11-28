#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


glm::vec3 Movement::getSimplePlayerDir(GLFWwindow* window)
{
    // returns normalized vector of player's movement based on simple WSAD inputs
    // if no inputs -> zero vector
    // x = right/left, y = up/down, z = backwards/forwards
    //TODO z == up/down 
    glm::vec3 move(0.f, 0.f, 0.f);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move.z -= 1.f;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move.z += 1.f;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move.x -= 1.f;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move.x += 1.f;

    return NORMALIZE_OR_0(move);
}
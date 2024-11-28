#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include <fstream>


bool Utils::isZero(glm::vec3 vector)
{
    return vector == glm::vec3(0.f);
}

size_t Utils::getTextFileLength(const char *path)
{
    assert(path != NULL);

    std::ifstream f(path, std::ios::in);
    if (!f.is_open()) return 0;

    size_t len = 0;
    while (f.get() != std::ifstream::traits_type::eof()) ++len;

    return len;
}

char* Utils::getTextFileAsString(const char *path)
{
    assert(path != NULL);

    size_t len = Utils::getTextFileLength(path);
    // printf("len: %d\n", len);
    if (!len) return NULL;

    std::ifstream f(path, std::ios::in);
    if (!f.is_open()) return NULL;
    
    char *result = new char[len + 1];

    f.read(result, len);
    if (f.gcount() != len)
    {
        delete[] result;
        return NULL;
    }

    result[len] = '\0'; //we need to add the terminating character

    return result;
}

GLint Utils::filteringEnumWithoutMipmap(GLint filtering)
{
    //IDEA I guess this could be more optimized using bitmasks
    switch (filtering)
    {
    case GL_NEAREST_MIPMAP_NEAREST: return GL_NEAREST;
    case GL_LINEAR_MIPMAP_NEAREST: return GL_LINEAR;
    case GL_NEAREST_MIPMAP_LINEAR: return GL_NEAREST;
    case GL_LINEAR_MIPMAP_LINEAR: return GL_LINEAR;
    }
    
    return filtering; // otherwise leave the value as it is
}

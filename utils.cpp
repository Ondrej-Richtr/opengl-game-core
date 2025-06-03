#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include <fstream>
#include "glm/trigonometric.hpp" // glm::cos, glm::sin


Utils::RNG::RNG(int min_val, int max_val)
                : m_generator(), m_distribution(min_val, max_val), m_distribution_circular(min_val, max_val - 1)
{
    std::random_device rd;
    m_generator = std::mt19937(rd());
}

int Utils::RNG::generate()
{
    return m_distribution(m_generator);
}

float Utils::RNG::generateFloatRange(float range_min, float range_max)
{
    assert(range_min <= range_max);

    int rolled = generate();

    float normalized = (float)(rolled - getMin()) / (getMax() - getMin()); // rolled number normalized to interval [0; 1]

    return normalized * (range_max - range_min) + range_min; // scaled to interval [range_min; range_max]
}

glm::vec2 Utils::RNG::generateAngledNormal(float angle_from, float angle_to)
{
    assert(angle_from <= angle_to);

    int rolled;
    if (angle_from == 0.f && angle_to == 2 * glm::pi<float>())
    {
        rolled = m_distribution_circular(m_generator);
    }
    else
    {
        rolled = generate();
    }

    // rolled number normalized to interval [0; 1) for angle range [0; 2pi)
    // and to [0; 1] for other ranges
    float normalized = (float)(rolled - getMin()) / (getMax() - getMin());

    // scaled to interval [angle_from; angle_to) for [0; 2pi] or to [angle_from; angle_to] for other ranges
    const float angle = normalized * (angle_to - angle_from) + angle_from;

    return glm::vec2(glm::cos(angle), glm::sin(angle)); // no need to normalize as cos^2(t) + sin^2(t) == 1 
}

int Utils::RNG::getMin() const
{
    return m_distribution.min();
}

int Utils::RNG::getMax() const
{
    return m_distribution.max();
}

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

char* Utils::getTextFileAsString_C_str(const char *path, size_t *result_len)
{
    // loads whole file as a C string, returns NULL when error
    assert(path != NULL);

    size_t len = Utils::getTextFileLength(path);
    // printf("len: %d\n", len);
    if (!len) return NULL;

    std::ifstream f(path, std::ios::in);
    if (!f.is_open()) return NULL;
    
    char *result = new char[len + 1]();

    f.read(result, len);
    if (f.gcount() != len)
    {
        delete[] result;
        return NULL;
    }

    result[len] = '\0'; //we need to add the terminating character

    if (result_len) *result_len = len;
    return result;
}

std::unique_ptr<char[]> Utils::getTextFileAsString(const char *path, size_t *result_len)
{
    // loads whole file as C string, caller takes ownership of allocated memory with returned unique_ptr,
    // returns NULL ptr when error
    return std::unique_ptr<char[]>(Utils::getTextFileAsString_C_str(path, result_len));
}

//std::string version does not seem like a good idea
/*std::string Utils::getTextFileAsString(const char *path)
{
    //TODO errors?
    // loads whole file as a std::string, returns empty string when error
    assert(path != NULL);

    size_t len = Utils::getTextFileLength(path);
    // printf("len: %d\n", len);
    if (!len) return std::string{};

    std::ifstream f(path, std::ios::in);
    if (!f.is_open()) return std::string{};
    
    //IDEA probably more idiomatic to use std::string with preallocated memory
    std::unique_ptr<char[]> result(new char[len + 1]);
    assert(result);

    f.read(result.get(), len);
    if (f.gcount() != len) return std::string{};

    result[len] = '\0'; //we need to add the terminating character

    return std::string(result.release());
}*/

/*GLint Utils::filteringEnumWithoutMipmap(GLint filtering)
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
}*/

glm::mat3 Utils::modelMatrixToNormalMatrix(const glm::mat4& model_mat)
{
    glm::mat3 result(model_mat);
    return glm::transpose(glm::inverse(result));
}

bool Utils::checkForGLError()
{
    // if no error -> return false
    if (glGetError() == GL_NO_ERROR) return false;

    // if error -> clear all GL error flags and return true
    while (glGetError() != GL_NO_ERROR);

    return true;
}

bool Utils::checkForGLErrorsAndPrintThem()
{
    bool ret = false;
    GLenum error = GL_NO_ERROR;

    while ((error = glGetError()) != GL_NO_ERROR)
    {
        printf("got error: 0x%x\n", error);
        ret = true;
    }

    return ret;
}

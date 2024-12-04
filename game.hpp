#pragma once

#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include <cstdio>
#include <cassert>
#include <cmath>
#include <array>

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720


//Macro functions
//TODO find a better solution than macros
#define NORMALIZE_OR_0(v) (Utils::isZero((v)) ? glm::vec3(0.f) : (v))

//struct definitions
struct ColorF
{
    GLfloat r = 0.f, g = 0.f, b = 0.f, a = 0.f;

    ColorF() = default;
    ColorF(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0)
            : r(r), g(g), b(b), a(a) {}
};

struct Color3F
{
    GLfloat r = 0.f, g = 0.f, b = 0.f;

    Color3F() = default;
    Color3F(GLfloat r, GLfloat g, GLfloat b)
            : r(r), g(g), b(b) {}
    Color3F(glm::vec3 color)
            : r(color.r), g(color.g), b(color.b) {}
};

struct Color
{
    unsigned char r = 0, g = 0, b = 0, a = 0;

    Color() = default;
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0)
            : r(r), g(g), b(b), a(a) {}

    ColorF toFloat() const
    {
        return ColorF{ (GLfloat)r / 255.f, (GLfloat)g / 255.f,
                       (GLfloat)b / 255.f, (GLfloat)a / 255.f };
    }
};

struct Material
{
    Color3F m_ambient, m_diffuse, m_specular;
    float m_shininess;

    Material() = default; //DEBUG
    Material(Color3F color, float shininess)
                : m_ambient(color), m_diffuse(color),
                  m_specular(0.5f, 0.5f, 0.5f), m_shininess(shininess) {}
};

struct LightSrc //TODO change this to props
{
    glm::vec3 m_pos;
    Color3F m_ambient, m_diffuse, m_specular;

    LightSrc(glm::vec3 pos, Color3F color, float ambient_intensity)
                : m_pos(pos), m_ambient(), m_diffuse(color), m_specular(1.f, 1.f, 1.f)
    {
        assert(ambient_intensity >= 0.f && ambient_intensity <= 1.f);
        m_ambient = Color3F(ambient_intensity * color.r, ambient_intensity * color.g, ambient_intensity * color.b);
    }
};

//TODO use this
// struct MainLoopData
// {
//     int test;
//     GLFWwindow* window;
// };

//drawing.cpp
namespace Drawing
{
    static const glm::vec3 up_dir = glm::vec3(0.f, 1.f, 0.f); // up is in the positive direction of Y axis

    struct Camera3D
    {
        glm::vec3 m_pos, m_target;
        glm::mat4 m_view_mat, m_proj_mat;

        Camera3D(float fov, float aspect_ration, glm::vec3 pos, glm::vec3 target,
                 float near_plane = 0.01f, float far_plane = 100.f); //TODO near/far plane defaults
        Camera3D(float fov, float aspect_ration, glm::vec3 pos, float pitch, float yaw,
                 float near_plane = 0.01f, float far_plane = 100.f); //TODO near/far plane defaults

        void setPosition(glm::vec3 pos);        // setter for camera position, in the future we might want to cache view matrix
        void movePosition(glm::vec3 move_vec);  // move camera position by given vector

        void setTarget(glm::vec3 target);       // setter for camera target, in the future we might want to cache view matrix
        void moveTarget(glm::vec3 move_vec);    // move camera target by given vector
        void setTargetFromPitchYaw(float pitch, float yaw); //TODO

        void move(glm::vec3 move_vec);          // combines movePosition and moveTarget
        
        void updateViewMatrix();

        const glm::mat4& getViewMatrix() const;

        const glm::mat4& getProjectionMatrix() const;

        glm::vec3 dirCoordsViewToWorld(glm::vec3 dir) const;
    };

    void clear(GLFWwindow* window, Color color);
};

namespace Utils
{
    bool isZero(glm::vec3 vector);

    size_t getTextFileLength(const char *path);
    char* getTextFileAsString(const char *path);

    GLint filteringEnumWithoutMipmap(GLint filtering);

    glm::mat3 modelMatrixToNormalMatrix(const glm::mat4& model_mat);
};

//shaders.cpp
namespace Shaders
{
    //TODO unite those ids?
    static const GLuint empty_id = 0; // id that is considered empty / invalid by OpenGL

    struct Program
    {
        GLuint m_id = empty_id; // OpenGL shader program id, by default the shader program has invalid (empty) id

        Program() = default;
        Program(GLuint vs_id, GLuint fs_id);
        Program(const char *vs_path, const char *fs_path);
        ~Program();

        void use() const;

        //void set(const char *uniform_name, std::array<float, 4> floats) const;
        void set(const char *uniform_name, Color3F color) const;
        void set(const char *uniform_name, glm::vec3 vec) const;
        void set(const char *uniform_name, glm::vec4 vec) const;
        void set(const char *uniform_name, GLint value) const;
        void set(const char *uniform_name, GLfloat value) const;
        void set(const char *uniform_name, const glm::mat3& matrix) const;
        void set(const char *uniform_name, const glm::mat4& matrix) const;

        void setMaterial(const Material& material) const;
        void setLightSrc(const LightSrc& light_src) const;
    };

    GLuint fromString(GLenum type, const char *str);

    GLuint programLink(GLuint vs, GLuint fs);

    void setupVertexAttribute_float(GLuint location, size_t count, size_t offset, size_t stride);

    void disableVertexAttribute(GLuint location);
}

//textures.cpp
namespace Textures
{
    //TODO unite those ids?
    static const GLuint empty_id = 0; // id that is considered empty / invalid by OpenGL

    // some default values for texture initialization,
    // currently there are no overrides for this as they are not needed yet
    static const GLint default_wrapping = GL_REPEAT;
    static const GLint default_min_filtering = GL_LINEAR_MIPMAP_LINEAR;
    static const GLint default_max_filtering = GL_LINEAR;

    struct Texture2D // struct representing an ingame texture with 4 channels (RGBA)
    {
        unsigned int m_id = empty_id; // OpenGL texture id
        unsigned int m_width = 0, m_height = 0;

        Texture2D() = default;
        Texture2D(const char *image_path, bool generate_mipmaps = true);
        ~Texture2D();

        void bind(unsigned int unit = 0) const;
    };
}

//movement.cpp
namespace Movement
{
    glm::vec3 getSimplePlayerDir(GLFWwindow* window);
}

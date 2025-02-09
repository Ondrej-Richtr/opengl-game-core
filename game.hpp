#pragma once

#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

//nuklear
//  both header and implementation defines
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "nuklear.h"

#include <cstdio>
#include <cassert>
#include <cmath>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <random>

#define FLOAT_TOLERANCE 0.001f

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define UNIFORM_NAME_BUFFER_LEN 512
// texbuffer length is set to exactly NK_INPUT_MAX as at worst each unicode character is 1 ascii character,
// if this memory is not enough then nuklear wouldnt store it anyways,
// this however does not mean that every possible state of our texbuffer would be successfuly loaded into nuklear buffer,
// a lot of written text per frame, even if stored in our buffer, might not fully work if nuklear buffer would be too small
#define UNICODE_TEXTBUFFER_LEN  NK_INPUT_MAX

#define UNIFORM_MATERIAL_NAME "material"
#define UNIFORM_MATERIAL_AMBIENT "ambient"
#define UNIFORM_MATERIAL_DIFFUSE "diffuse"
#define UNIFORM_MATERIAL_SPECULAR "specular"
#define UNIFORM_MATERIAL_SHININESS "shininess"

#define UNIFORM_LIGHTPROPS_ATTRNAME "props"
#define UNIFORM_LIGHTPROPS_AMBIENT "ambient"
#define UNIFORM_LIGHTPROPS_DIFFUSE "diffuse"
#define UNIFORM_LIGHTPROPS_SPECULAR "specular"

#define UNIFORM_LIGHT_NAME "lights"
#define UNIFORM_LIGHT_TYPE "type"
#define UNIFORM_LIGHT_ATTENUATION "atten_coefs"
#define UNIFORM_LIGHT_POSITION "pos"
#define UNIFORM_LIGHT_DIRECTION "dir"
#define UNIFORM_LIGHT_COSINNERCUTOFF "cosInnerCutoff"
#define UNIFORM_LIGHT_COSOUTERCUTOFF "cosOuterCutoff"

#define UNIFORM_LIGHT_COUNT_NAME "lightsCount"

//Macro functions
// returns normalized vector or zero vector if the given vector is zero
//TODO find a better solution than macros
#define NORMALIZE_OR_0(v) (Utils::isZero((v)) ? glm::vec3(0.f) : (v))
// returns size_t length of string (must be string literal or char array with term. char.),
// -1 as we dont count the term. char.
#define STR_LEN(S) ((sizeof((S)) / sizeof((S)[0])) - 1)
// returns whether given float number is close to zero according to FLOAT_TOLERANCE macro
#define CLOSE_TO_0(n) ((n) <= FLOAT_TOLERANCE && (n) >= -FLOAT_TOLERANCE)


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

namespace Lighting
{
    class Light;
};

namespace Shaders
{
    struct Program;
};

namespace Textures
{
    struct Texture2D;
};

namespace Game
{
    class Target;
};

namespace Collision
{
    struct Ray;
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

        glm::vec3 getDirection() const;

        Collision::Ray getRay() const;
    };

    struct FrameBuffer
    {
        enum class AttachmentType { none, render, texture };
        struct Attachment
        {
            GLuint id;
            AttachmentType type;
        };

        GLuint m_id;

        FrameBuffer();
        ~FrameBuffer();

        void bind() const;
        void unbind() const;

        void attachColorBuffer(Attachment attachment) const;
        void attachDepthBuffer(Attachment attachment) const;
        void attachStencilBuffer(Attachment attachment) const;

        void attachAll(Attachment color, Attachment depth, Attachment stencil) const;

        bool isComplete() const;
    };

    void clear(Color color);

    void texturedRectangle(const Shaders::Program& tex_rect_shader, const Textures::Texture2D& textureRect,
                           glm::vec2 dstPos, glm::vec2 dstSize);

    void screenLine(const Shaders::Program& line_shader, unsigned int line_vbo, glm::vec2 screen_res,
                    glm::vec2 v1, glm::vec2 v2, float thickness, ColorF color);

    void crosshair(const Shaders::Program& line_shader, unsigned int line_vbo, glm::vec2 screen_res,
                   glm::vec2 size, glm::vec2 screen_pos, float thickness, ColorF color);
    
    void target(const Shaders::Program& shader, const Drawing::Camera3D& camera,
                const std::vector<std::reference_wrapper<const Lighting::Light>>& lights, const Game::Target& target,
                double current_frame_time, glm::vec3 pos_offset = glm::vec3(0.f));
}

//lighting.cpp
namespace Lighting
{
    struct MaterialProps //should correspond to Material struct in shaders
    {
        Color3F m_ambient, m_diffuse, m_specular;
        float m_shininess;

        MaterialProps() = default; //DEBUG
        MaterialProps(Color3F color, float shininess)
                        : m_ambient(color), m_diffuse(color),
                        m_specular(0.5f, 0.5f, 0.5f), m_shininess(shininess) {}
    };

    struct LightProps //should correspond to LightProps struct in shaders
    {
        Color3F m_ambient, m_diffuse, m_specular;

        LightProps(Color3F color, float ambient_intensity)
                    : m_ambient(), m_diffuse(color), m_specular(1.f, 1.f, 1.f)
        {
            assert(ambient_intensity >= 0.f && ambient_intensity <= 1.f);
            m_ambient = Color3F(ambient_intensity * color.r, ambient_intensity * color.g, ambient_intensity * color.b);
        }
    };

    // Lights - directional (dir vec), point (pos vec), spot (dir vec, pos vec, inner/outer cone cutoff angle)
    static const size_t lights_max_amount = 10; //TODO make this synchronized with LIGHTS_MAX_AMOUNT in light.fs fragent shader!

    class Light //abstract class representing singular light source (directional/point/spot light)
    {
    public:
        enum class Type { directional = 0, point = 1, spot = 2 };

        LightProps m_props;

        Light(const LightProps& props);
        virtual ~Light() = default;

        bool bindPropsToShader(const char *uniform_name, const Shaders::Program& shader, int idx = -1) const;

        virtual bool bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx = -1) const = 0;
    };

    class DirLight : public Light
    {
    public:
        glm::vec3 m_dir;

        DirLight(const LightProps& props, glm::vec3 dir);
        ~DirLight() = default;

        bool bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx = -1) const override;
    };

    class PointLight : public Light
    {
    public:
        glm::vec3 m_pos;

        //TODO coef values, see https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        GLfloat m_attenuation_coefs_const = 1.f; // constant coeficient used to calculate attenuation value
        GLfloat m_attenuation_coefs_lin   = 0.f; // linear coeficient used to calculate attenuation value
        GLfloat m_attenuation_coefs_quad  = 0.f; // quadratic coeficient used to calculate attenuation value

        PointLight(const LightProps& props, glm::vec3 pos);
        ~PointLight() = default;

        bool bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx = -1) const override;

        void setAttenuation(GLfloat constant, GLfloat linear, GLfloat quadratic);
    };

    class SpotLight : public Light
    {
    public:
        glm::vec3 m_dir;
        glm::vec3 m_pos;
        float m_cos_in_cutoff;  // cosine of inner cutoff angle - defines area with full light intensity
        float m_cos_out_cutoff; // cosine of outer cutoff angle - defines area that makes light smoothly fade at the edges

        //TODO coef values, see https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        float m_attenuation_coefs_const = 1.f; // constant coeficient used to calculate attenuation value
        float m_attenuation_coefs_lin   = 0.f; // linear coeficient used to calculate attenuation value
        float m_attenuation_coefs_quad  = 0.f; // quadratic coeficient used to calculate attenuation value

        SpotLight(const LightProps& props, glm::vec3 dir, glm::vec3 pos,
                  float inner_cutoff_angle, float outer_cutoff_angle);
        ~SpotLight() = default;

        bool bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx = -1) const override;

        void setAttenuation(GLfloat const, GLfloat linear, GLfloat quadratic);
    };
}

namespace Utils
{
    class RNG
    {
        std::mt19937 m_generator; // mersenne_twister_engine
        std::uniform_int_distribution<int> m_distribution;

    public:
        RNG(int min_val, int max_val);
        ~RNG() = default;

        int generate();
        float generateFloatRange(float range_min, float range_max);

        int getMin() const;
        int getMax() const;
    };

    bool isZero(glm::vec3 vector);

    size_t getTextFileLength(const char *path);
    char* getTextFileAsString_C_str(const char *path);
    std::unique_ptr<char[]> getTextFileAsString(const char *path);

    constexpr GLint filteringEnumWithoutMipmap(GLint filtering)
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

    glm::mat3 modelMatrixToNormalMatrix(const glm::mat4& model_mat);

    bool checkForGLError();
};

//shaders.cpp
namespace Shaders
{
    //TODO unite those ids?
    static const GLuint empty_id = 0; // id that is considered empty / invalid by OpenGL

    static const GLuint attribute_position_verts = 0;
    static const GLuint attribute_position_texcoords = 1;
    static const GLuint attribute_position_normals = 2;
    static const GLuint attribute_position_color = 3;

    struct Program
    {
        GLuint m_id = empty_id; // OpenGL shader program id, by default the shader program has invalid (empty) id

        Program() = default;
        Program(GLuint vs_id, GLuint fs_id);
        Program(const char *vs_path, const char *fs_path);
        ~Program();

        void use() const;

        //void set(const char *uniform_name, std::array<float, 4> floats) const;
        void set(const char *uniform_name, ColorF color) const;
        void set(const char *uniform_name, Color3F color) const;
        void set(const char *uniform_name, glm::vec2 vec) const;
        void set(const char *uniform_name, glm::vec3 vec) const;
        void set(const char *uniform_name, glm::vec4 vec) const;
        void set(const char *uniform_name, GLint value) const;
        void set(const char *uniform_name, GLfloat value) const;
        void set(const char *uniform_name, const glm::mat3& matrix) const;
        void set(const char *uniform_name, const glm::mat4& matrix) const;

        void setMaterialProps(const Lighting::MaterialProps& material) const;
        bool setLight(const char *uniform_name, const Lighting::Light& light, int idx = -1) const;
        int setLights(const char *uniform_array_name, const char *uniform_arrray_size_name,
                      const std::vector<std::reference_wrapper<const Lighting::Light>>& lights) const;
    };

    GLuint fromString(GLenum type, const char *str);

    GLuint programLink(GLuint vs, GLuint fs);

    void setupVertexAttribute_float(GLuint location, size_t count, size_t offset, size_t stride, bool offset_in_bytes = false);
    void setupVertexAttribute_ubyte(GLuint location, size_t count, size_t offset, size_t stride, bool offset_in_bytes = false);

    void disableVertexAttribute(GLuint location);

    //basic static shader programs
    // IMPORTANT: needs to get initialized first by calling initBasicShaderPrograms!
    // bool initBasicShaderPrograms();
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
        Texture2D(unsigned int width, unsigned int height, GLenum component_type);
        Texture2D(const char *image_path, bool generate_mipmaps = true);
        Texture2D(const void *img_data, unsigned int width, unsigned int height,
                  bool generate_mipmaps = true);
        ~Texture2D();

        void bind(unsigned int unit = 0) const;

        Drawing::FrameBuffer::Attachment asFrameBufferAttachment() const;
    };
}

//meshes.cpp
namespace Meshes
{
    //TODO meshes - verts, normals, texcoords, indices for faces, (material?)

    //TODO unite those ids?
    static const GLuint empty_id = 0; // id that is considered empty / invalid by OpenGL

    static const size_t attribute_verts_amount = 3;     // vec3
    static const size_t attribute_texcoord_amount = 2;  // vec2
    static const size_t attribute_normal_amount = 3;    // vec3

    //Vertex buffer object abstraction, should be used mainly for mesh data
    //  consists of (in this order) - vertex positions, vertex texture coordinates (optional), vertex normals (optional)
    struct VBO
    {
        GLuint m_id = empty_id;

        size_t m_vert_count;    // amount of vertices that this vbo holds
        size_t m_stride;        // stride in bytes (vec3 position + (optional) vec2 texcoords + (optional) vec3 normal)
        int m_texcoord_offset, m_normal_offset; // offsets into the vbo NOT in bytes, -1 means that attribute is not included

        VBO(); // default constructor for uninitialized VBO
        VBO(const GLfloat *data, size_t data_vert_count, bool texcoords, bool normals);
        ~VBO();

        VBO& operator=(VBO&& other); // this is sadly needed because of global meshes and generate functions

        void bind() const;
        void unbind() const;
    };

    //style of UV texcoords
    //  none    - no texcoords
    //  stretch - fit each face into 0.0-1.0 UV coordinates
    //  repeat  - set repeating according to texture size (size in world coordinates)
    enum class TexcoordStyle { none, stretch, repeat };

    Meshes::VBO generateCubicVBO(glm::vec3 mesh_scale,  glm::vec2 texture_world_size,
                                 Meshes::TexcoordStyle style, bool normals);
    
    Meshes::VBO generateQuadVBO(glm::vec2 mesh_scale, glm::vec2 texture_world_size,
                                Meshes::TexcoordStyle style, bool normals);
    
    //basic global meshes
    // IMPORTANT: needs to get initialized first by calling initBasicMeshes!
    extern VBO unit_quad_pos_only;
    // extern VBO unit_quad_pos_uv_only;

    bool initBasicMeshes();
}

//movement.cpp
namespace Movement
{
    glm::vec3 getSimplePlayerDir(GLFWwindow* window);
}

//collision.cpp
namespace Collision
{
    struct Ray
    {
        glm::vec3 m_pos, m_dir;

        Ray(glm::vec3 pos, glm::vec3 dir);
        ~Ray() = default;
    };

    struct RayCollision
    {
        bool m_hit = false;
        float m_travel = 0.f; // distance that ray had to travel to make this collision
        glm::vec3 m_point = glm::vec3(0.f);

        RayCollision() = default;
        RayCollision(float travel, glm::vec3 point);
        ~RayCollision() = default;
    };

    RayCollision rayPlane(Ray ray, glm::vec3 plane_normal, glm::vec3 plane_pos);

    RayCollision rayTarget(Ray ray, glm::vec3 target_normal, glm::vec3 target_pos, float target_radius);
}

namespace UI
{
    struct Vertex
    {
        GLfloat pos[2]; // important to keep it to 2 floats
        GLfloat uv[2];
        GLubyte color[4];
    };

    struct Font
    {
    private:
        nk_font_atlas m_atlas;
        nk_font *m_font;
    public:
        nk_draw_null_texture m_null_texture;
        Textures::Texture2D m_texture;

        Font(const char *font_path, float font_height);
        ~Font();

        const nk_user_font* getFontPtr() const; 
    };

    struct Context
    {
        // static const size_t max_vertex_memory = 512 * 1024;
        // static const size_t max_element_memory = 128 * 1024;

        nk_context m_ctx;
        bool m_ctx_initialized;
        nk_convert_config m_cfg;
        nk_buffer m_cmd_buffer, m_vert_buffer, m_idx_buffer;

        const Shaders::Program& m_shader;
        GLuint m_vbo_id, m_ebo_id;

        Context(const Shaders::Program& shader, const UI::Font& font);
        ~Context();

        bool getInput(GLFWwindow* window, glm::vec2 mouse_pos, bool mouse_left_is_down,
                      unsigned int textbuffer[], size_t textbuffer_len);

        bool convert();

        bool draw(glm::vec2 screen_res, unsigned int texture_unit = 0);

        void clear();
    };
}

//game.hpp
namespace Game
{
    class Target
    {
    public:
        constexpr static const float size_min = 0.1f, size_max = 0.5f;
        constexpr static const float grow_time = 2.5f; // 2.5 seconds

        const Meshes::VBO& m_vbo;
        const Textures::Texture2D& m_texture;
        const Lighting::MaterialProps& m_material;

        glm::vec3 m_pos;
        double m_spawn_time;

        Target(const Meshes::VBO& vbo, const Textures::Texture2D& texture, const Lighting::MaterialProps& material,
               glm::vec3 pos, double spawn_time);
        Target(const Target& other);
        ~Target() = default;

        Target& operator=(const Target& other);

        static glm::vec3 generateXZPosition(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size);

        glm::vec2 getSize(double time) const;

        void draw(const Shaders::Program& shader, const Drawing::Camera3D& camera,
                  const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                  double current_frame_time, glm::vec3 pos_offset = glm::vec3(0.f)) const;
    };
}

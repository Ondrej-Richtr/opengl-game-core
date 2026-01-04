#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/mat3x3.hpp" // IWYU pragma: keep
#include "glm/mat4x4.hpp" // IWYU pragma: keep
#include "glm/vec2.hpp" // IWYU pragma: keep
#include "glm/vec3.hpp" // IWYU pragma: keep
#include "glm/ext/scalar_constants.hpp" // glm::pi

//nuklear
// both header and implementation defines
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "nuklear.h"

#include <cstdio>
#include <cassert>
#include <cmath> // IWYU pragma: keep
#include <array> // IWYU pragma: keep
#include <vector>
#include <memory>
#include <functional>
#include <random>
#include <optional>
#include <variant>

#define FLOAT_TOLERANCE 0.001f

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define ATTRIBUTE_DEFAULT_NAME_POS "aPos"
#define ATTRIBUTE_DEFAULT_NAME_TEXCOORDS "aTexCoord"
#define ATTRIBUTE_DEFAULT_NAME_NORMALS "aNormal"
#define ATTRIBUTE_DEFAULT_NAME_COLOR "aColor"

// maximal length of a uniform name/location
//TODO WebGL imposes limit of 256, maybe change to that?
#define UNIFORM_NAME_BUFFER_LEN 512
// texbuffer length is set to exactly NK_INPUT_MAX as at worst each unicode character is 1 ascii character,
// if this memory is not enough then nuklear wouldnt store it anyways,
// this however does not mean that every possible state of our texbuffer would be successfuly loaded into nuklear buffer,
// a lot of written text per frame, even if stored in our buffer, might not fully work if nuklear buffer would be too small
#define UNICODE_TEXTBUFFER_LEN  NK_INPUT_MAX

#define UNIFORM_MATERIAL_NAME "material"
#define UNIFORM_MATERIAL_AMBIENT "ambient"
#define UNIFORM_MATERIAL_DIFFUSE "diffuse"
#define UNIFORM_MATERIAL_DIFFUSE_MAP "diffuseMap"
#define UNIFORM_MATERIAL_DIFFUSE_MAP_UNIT 0
#define UNIFORM_MATERIAL_SPECULAR "specular"
#define UNIFORM_MATERIAL_SPECULAR_MAP "specularMap"
#define UNIFORM_MATERIAL_SPECULAR_MAP_UNIT 1
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
//TODO there is no normalization?
#define NORMALIZE_OR_0(v) (Utils::isZero((v)) ? glm::vec3(0.f) : (glm::normalize((v))))
// returns size_t length of string (must be string literal or char array with term. char.),
// -1 as we dont count the term. char.
#define STR_LEN(S) ((sizeof((S)) / sizeof((S)[0])) - 1)
// returns whether given float number is close to zero according to FLOAT_TOLERANCE macro
#define CLOSE_TO_0(n) ((n) <= FLOAT_TOLERANCE && (n) >= -FLOAT_TOLERANCE)
// returns whether two given float numbers are close to eachother according to CLOSE_TO_0 macro
#define FLOAT_EQUALS(a, b) (CLOSE_TO_0((a) - (b)))

//constants
constexpr GLuint empty_id = 0;


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
    constexpr Color3F(GLfloat r, GLfloat g, GLfloat b)
            : r(r), g(g), b(b) {}
    Color3F(GLfloat val)
            : r(val), g(val), b(val) {}
    Color3F(glm::vec3 color)
            : r(color.r), g(color.g), b(color.b) {}
    Color3F(const GLfloat rgb[])
            : r(rgb[0]), g(rgb[1]), b(rgb[2]) {}
    
    glm::vec3 toVec() const { return glm::vec3(r, g, b); }

    Color3F mult(const Color3F other) const { return toVec() * other.toVec(); }

    Color3F scalarMult(GLfloat scalar) const { return scalar * toVec(); }
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

struct Color3
{
    unsigned char r = 0, g = 0, b = 0;

    Color3() = default;
    Color3(unsigned char r, unsigned char g, unsigned char b)
            : r(r), g(g), b(b) {}
};

namespace Drawing
{
    struct Camera3D;

    struct FrameBuffer;
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
    struct Cubemap;
};

namespace Meshes
{
    struct VBO;
};

namespace Game
{
    class Target;
};

namespace Collision
{
    struct Ray;
};

//window_manager.cpp
class WindowManager
{
    static GLFWwindow *m_window;
    static glm::ivec2 m_win_size, m_framebuffer_size;

public:
    static void windowResizeCallback(GLFWwindow* window, int width, int height);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static void init(GLFWwindow *window);

    static GLFWwindow* getWindow();

    static glm::ivec2 getSize();
    static glm::vec2 getSizeF();

    static glm::ivec2 getFBOSize();
    static glm::vec2 getFBOSizeF(); 
};

//mouse_manager.cpp
class MouseManager
{
    //TODO save if mouse_pos was set atleast from within the callback
    static glm::vec2 mouse_pos;

public:
    static bool left_button, right_button;

    static void init(GLFWwindow *window);

    static glm::ivec2 mousePos();
    static glm::ivec2 mousePosF();

    static void setCursorLocked();
    static void setCursorVisible();

private:
    static void mousePositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonsCallback(GLFWwindow *window, int button, int action, int mods);
};

//drawing.cpp
namespace Drawing
{
    static constexpr glm::vec3 up_dir = glm::vec3(0.f, 1.f, 0.f); // up is in the positive direction of Y axis

    static constexpr float default_near_plane = 0.01f, default_far_plane = 100.f;

    struct Camera3D
    {
        glm::vec3 m_pos, m_target;
        glm::mat4 m_view_mat, m_proj_mat;

        Camera3D(float fov, float aspect_ratio, glm::vec3 pos, glm::vec3 target,
                 float near_plane = default_near_plane, float far_plane = default_far_plane);
        Camera3D(float fov, float aspect_ratio, glm::vec3 pos, float pitch, float yaw,
                 float near_plane = default_near_plane, float far_plane = default_far_plane);

        void setPosition(glm::vec3 pos);        // setter for camera position, in the future we might want to cache view matrix
        void movePosition(glm::vec3 move_vec);  // move camera position by given vector

        void setTarget(glm::vec3 target);       // setter for camera target, in the future we might want to cache view matrix
        void moveTarget(glm::vec3 move_vec);    // move camera target by given vector
        void setTargetFromPitchYaw(float pitch, float yaw);

        void move(glm::vec3 move_vec);          // combines movePosition and moveTarget
        
        void updateViewMatrix();

        void setProjectionMatrix(float fov, float aspect_ratio,
                                 float near_plane = default_near_plane, float far_plane = default_far_plane);

        const glm::mat4& getViewMatrix() const;

        const glm::mat4& getProjectionMatrix() const;

        glm::vec3 dirCoordsViewToWorld(glm::vec3 dir) const;

        glm::vec3 getDirection() const;

        Collision::Ray getRay() const;
    };

    #ifndef USE_COMBINED_FBO_BUFFERS
        // This might (and should) seem weird as we typically either use OpenGL 3.3 for desktop or OpenGLES 2.0 for the web,
        // meaning that the following condition would be always true.
        // But in the future there might be some specific scenario when we want to use OpenGLES 2.0 even on desktops. (Old machines? Phones?)
        #if defined(BUILD_OPENGL_330_CORE) || defined(PLATFORM_WEB)
            // Combine depth and stencil buffer together - only possible on newer desktop OpenGL or in WebGL (not on ES).
            #define USE_COMBINED_FBO_BUFFERS
        #endif
    #endif

    struct FrameBuffer
    {
        enum class AttachmentType { none, render, texture, textureMultiSample };
        struct Attachment
        {
            GLuint id;
            AttachmentType type;
        };

        GLuint m_id = empty_id;

        FrameBuffer() = default;
        FrameBuffer(bool dummy);
        ~FrameBuffer();

        void init();
        void deinit();

        void bind() const;
        void unbind() const; //TODO refactor? unbinds are an OpenGL anti-pattern

        void attachColorBuffer(Attachment attachment) const;
        void attachDepthBuffer(Attachment attachment) const;
        #ifdef USE_COMBINED_FBO_BUFFERS
            void attachDepthStencilBuffer(Attachment attachment) const;

            void attachAllCombined(Attachment color, Attachment depthStencil) const;
        #else
            void attachStencilBuffer(Attachment attachment) const;
        #endif        
        void attachAllSeparated(Attachment color, Attachment depth, Attachment stencil) const;

        bool isComplete() const;
    };

    Color3F blendScreen(Color3F a, Color3F b);

    void clear(Color color);

    void texturedRectangle(const Shaders::Program& tex_rect_shader, const Textures::Texture2D& textureRect,
                           glm::vec2 screen_res, glm::vec2 dstPos, glm::vec2 dstSize);

    void texturedRectangle2(const Shaders::Program& tex_rect_shader, const Textures::Texture2D& textureRect,
                            const Textures::Texture2D& background, const Textures::Texture2D& foreground,
                            glm::vec2 dstPos, glm::vec2 dstSize);

    void screenLine(const Shaders::Program& line_shader, const Meshes::VBO& line_vbo, glm::vec2 screen_res,
                    glm::vec2 v1, glm::vec2 v2, float thickness, ColorF color);

    void crosshair(const Shaders::Program& line_shader, const Meshes::VBO& line_vbo, glm::vec2 screen_res,
                   glm::vec2 size, glm::vec2 screen_pos, float thickness, ColorF color);
}

//lighting.cpp
namespace Lighting
{
    struct MaterialProps //should correspond to Material struct in shaders
    {
        Color3F m_ambient, m_diffuse, m_specular;
        float m_shininess;

        MaterialProps(Color3F ambient, Color3F diffuse, Color3F specular, float shininess)
                        : m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) {}
        MaterialProps(Color3F color, float shininess)
                        : m_ambient(color), m_diffuse(color),
                        m_specular(0.5f, 0.5f, 0.5f), m_shininess(shininess) {}
    };

    struct Material
    {
        MaterialProps m_props;
        const Textures::Texture2D &m_diffuse_map, &m_specular_map;

        Material(MaterialProps props, const Textures::Texture2D& diffuse_map, const Textures::Texture2D& specular_map)
                    : m_props(props), m_diffuse_map(diffuse_map), m_specular_map(specular_map) {}
    };

    struct LightProps //should correspond to LightProps struct in shaders
    {
        Color3F m_ambient, m_diffuse, m_specular;

        LightProps(Color3F ambient, Color3F diffuse, Color3F specular)
                    : m_ambient(ambient), m_diffuse(diffuse), m_specular(specular) {}

        LightProps(Color3F color, float ambient_intensity)
                    : m_ambient(), m_diffuse(color), m_specular(0.5f, 0.5f, 0.5f)
        {
            assert(ambient_intensity >= 0.f && ambient_intensity <= 1.f);
            m_ambient = Color3F(ambient_intensity * color.r, ambient_intensity * color.g, ambient_intensity * color.b);
        }
    };

    // Lights - directional (dir vec), point (pos vec), spot (dir vec, pos vec, inner/outer cone cutoff angle)
    constexpr size_t lights_max_amount = 10; //TODO make this synchronized with light shader includes!
    constexpr float light_src_size = 0.2f;

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

        void setAttenuation(GLfloat constant, GLfloat linear, GLfloat quadratic);
    };
}

namespace Utils
{
    class RNG
    {
        std::mt19937 m_generator; // mersenne_twister_engine
        std::uniform_int_distribution<int> m_distribution;
        std::uniform_int_distribution<int> m_distribution_circular; // just with max_val-1

    public:
        RNG(int min_val, int max_val);
        ~RNG() = default;

        int generate();
        float generateFloatRange(float range_min, float range_max);
        glm::vec2 generateAngledNormal(float angle_from = 0.f, float angle_to = 2 * glm::pi<float>());

        int getMin() const;
        int getMax() const;
    };

    bool isZero(glm::vec3 vector);

    size_t getTextFileLength(const char *path);
    char* getTextFileAsString_C_str(const char *path, size_t *result_len);
    std::unique_ptr<char[]> getTextFileAsString(const char *path, size_t *result_len);

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

    glm::mat3 stripTranslationFromMatrix(const glm::mat4& mat);
    
    glm::mat3 modelMatrixToNormalMatrix(const glm::mat4& model_mat);

    bool checkForGLError();
    bool checkForGLErrorsAndPrintThem();
};

//shaders.cpp
namespace Shaders
{
    #ifndef SHADERS_DIR_PATH
        #define SHADERS_DIR_PATH "shaders/"
    #endif

    #ifndef SHADERS_PARTIALS_DIR_PATH
        #define SHADERS_PARTIALS_DIR_PATH SHADERS_DIR_PATH "partials/"
    #endif

    #ifndef USE_VER100_SHADERS
        #ifndef BUILD_OPENGL_330_CORE
            #define USE_VER100_SHADERS
        #endif
    #endif

    #ifdef USE_VER100_SHADERS
        #define SHADER_VER_LINE "#version 100\n"
        #define SHADER_ATTR_DEFINES_VS "#define IN_ATTR attribute\n#define OUT_ATTR varying\n"
        #define SHADER_ATTR_DEFINES_FS "#define IN_ATTR varying\n"\
                                       "#define TEXTURE2D(s,c) (texture2D((s), (c)))\n#define TEXTURECUBE(s,c) (textureCube((s), (c)))\n"\
                                       "#define GAMMA(c,g) ((g) == 0.0 ? (c) : vec4(pow((c).rgb, vec3(g)), (c).a))\n#define TEXTURE2DGAMMA(s,c) GAMMA(TEXTURE2D(s,c), (gammaCoef))\n#define TEXTURECUBEGAMMA(s,c) GAMMA(TEXTURECUBE(s,c), (gammaCoef))\n"\
                                       "#define OUTPUT_COLOR(c) (gl_FragColor = (c))\n"\
                                       "#define GAMMACORRECTION(c,g) ((g) == 0.0 ? (c) : vec4(pow((c).rgb, vec3(1.0/(g))), (c).a))\n#define OUTPUT_COLOR_GAMMA_CORRECTED(c) OUTPUT_COLOR(GAMMACORRECTION(c, (gammaCoef)))\n"
    #else
        #define SHADER_VER_LINE "#version 330 core\n"
        #define SHADER_ATTR_DEFINES_VS "#define IN_ATTR in\n#define OUT_ATTR out\n"
        #define SHADER_ATTR_DEFINES_FS "#define IN_ATTR in\n"\
                                       "#define TEXTURE2D(s,c) (texture((s), (c)))\n#define TEXTURECUBE(s,c) (texture((s), (c)))\n"\
                                       "#define GAMMA(c,g) ((g) == 0.0 ? (c) : vec4(pow((c).rgb, vec3(g)), (c).a))\n#define TEXTURE2DGAMMA(s,c) GAMMA(TEXTURE2D(s,c), (gammaCoef))\n#define TEXTURECUBEGAMMA(s,c) GAMMA(TEXTURECUBE(s,c), (gammaCoef))\n"\
                                       "out vec4 FragColor;\n#define OUTPUT_COLOR(c) (FragColor = (c))\n"\
                                       "#define GAMMACORRECTION(c,g) ((g) == 0.0 ? (c) : vec4(pow((c).rgb, vec3(1.0/(g))), (c).a))\n#define OUTPUT_COLOR_GAMMA_CORRECTED(c) OUTPUT_COLOR(GAMMACORRECTION(c, (gammaCoef)))\n"
    #endif

    #define SHADER_VER_INCLUDE_LINES_VS SHADER_VER_LINE SHADER_ATTR_DEFINES_VS
    #define SHADER_VER_INCLUDE_LINES_FS SHADER_VER_LINE SHADER_ATTR_DEFINES_FS

    //TODO because of support for ver100 we need to calculate those positions after compiling the shader
    constexpr GLuint attribute_position_pos = 0;
    constexpr GLuint attribute_position_texcoords = 1;
    constexpr GLuint attribute_position_normals = 2;
    constexpr GLuint attribute_position_color = 3;

    struct IncludeDefine
    {
        static constexpr size_t include_buffer_capacity = 512 * 1024; //TODO maybe dont store large buffers on the stack

        const char *m_name, *m_value;

        IncludeDefine(const char *name, const char *value = NULL);
    };

    struct ShaderInclude
    {
        bool is_define;
        union 
        {
            const char *str;
            IncludeDefine define;
        };

        ShaderInclude(const char *str);
        ShaderInclude(IncludeDefine&& define);
    };

    struct Program
    {
        GLuint m_id = empty_id; // OpenGL shader program id, by default the shader program has invalid (empty) id

        Program() = default;
        Program(GLuint vs_id, GLuint fs_id);
        Program(const char *vs_path, const char *fs_path,
                const std::vector<ShaderInclude>& vs_includes = {}, const std::vector<ShaderInclude>& fs_includes = {});
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

        void bindTexture(const char *sampler2d_name, const Textures::Texture2D& texture, unsigned int unit = 0) const;
        void bindCubemap(const char *samplerCube_name, const Textures::Cubemap& cubemap, unsigned int unit = 0) const;
        void bindDiffuseMap(const Textures::Texture2D& diffuse_map, int map_bind_offset = 0) const;
        void bindSpecularMap(const Textures::Texture2D& specular_map, int map_bind_offset = 0) const;

        void setMaterialProps(const Lighting::MaterialProps& material_props) const;
        void setMaterial(const Lighting::Material& material, int map_bind_offset = 0) const;
        bool setLight(const char *uniform_name, const Lighting::Light& light, int idx = -1) const;
        int setLights(const char *uniform_array_name, const char *uniform_arrray_size_name,
                      const std::vector<std::reference_wrapper<const Lighting::Light>>& lights) const;
    };

    GLuint fromString(GLenum type, const char *src);
    GLuint fromStringWithIncludeSystem(GLenum type, const char *src, const std::vector<ShaderInclude>& includes);

    GLuint programLink(GLuint vs, GLuint fs);

    void setupVertexAttribute_float(GLuint location, size_t count, size_t offset, size_t stride, bool offset_in_bytes = false);
    void setupVertexAttribute_ubyte(GLuint location, size_t count, size_t offset, size_t stride, bool offset_in_bytes = false);

    void disableVertexAttribute(GLuint location);
}

//textures.cpp
namespace Textures
{
    // some default values for texture initialization,
    // currently there are no overrides for this as they are not needed yet
    static constexpr GLint default_wrapping = GL_REPEAT;
    static constexpr GLint default_min_filtering = GL_LINEAR_MIPMAP_LINEAR;
    static constexpr GLint default_max_filtering = GL_LINEAR;

    constexpr bool default_generate_mipmaps = true;

    struct Texture2D // struct representing an ingame texture with 4 channels (RGBA)
    {
        unsigned int m_id = empty_id; // OpenGL texture id
        unsigned int m_width = 0, m_height = 0;
        unsigned int m_samples = 0;

        Texture2D() = default;
        Texture2D(unsigned int width, unsigned int height, GLenum component_type, unsigned int samples = 1);
        Texture2D(const char *image_path, bool generate_mipmaps = default_generate_mipmaps);
        Texture2D(const void *img_data, unsigned int width, unsigned int height,
                  bool generate_mipmaps = default_generate_mipmaps);
        Texture2D(Color3 color);
        ~Texture2D();

        GLenum getBindType() const;
        void bind(unsigned int unit = 0) const;

        bool isMultiSampled() const;

        void changeTexture(unsigned int new_width, unsigned int new_height,
                           GLenum component_type = GL_RGBA, const void *new_data = NULL);
        void changeTextureToPixel(Color3 color);

        bool copyContentsFrom(const Drawing::FrameBuffer& fbo_src, unsigned int width, unsigned int height, GLenum format = GL_RGBA);

        Drawing::FrameBuffer::Attachment asFrameBufferAttachment() const;
    };

    // some default values for cubemap initialization,
    // currently there are no overrides for this as they are not needed yet
    static constexpr GLint cubemap_default_wrapping = GL_CLAMP_TO_EDGE;
    static constexpr GLint cubemap_default_min_filtering = GL_LINEAR_MIPMAP_LINEAR;
    static constexpr GLint cubemap_default_max_filtering = GL_LINEAR;

    struct Cubemap
    {
        unsigned int m_id = empty_id; // OpenGL texture id
        std::array<unsigned int, 6> m_size_per_face = { 0, 0, 0, 0, 0, 0 };

        Cubemap() = default;
        ~Cubemap();
        
        void bind(unsigned int unit = 0) const;

        void createEmpty(unsigned int width_height, GLenum component_type, bool generate_mipmaps);

        void createFrom6Images(const std::array<const char*, 6>& image_paths, bool generate_mipmaps);
    };
}

//meshes.cpp
namespace Meshes
{
    #ifndef USE_VAO
        #ifdef BUILD_OPENGL_330_CORE
            #define USE_VAO
        #endif
    #endif

    constexpr unsigned int attribute3d_pos_amount = 3;       // vec3
    constexpr unsigned int attribute3d_texcoord_amount = 2;  // vec2
    constexpr unsigned int attribute3d_normal_amount = 3;    // vec3
    constexpr unsigned int attribute3d_complete_amount = attribute3d_pos_amount +
                                                         attribute3d_texcoord_amount +
                                                         attribute3d_normal_amount;

    constexpr unsigned int attribute2d_pos_amount = 2;       // vec2
    // constexpr unsigned int attribute2d_texcoord_amount = 2;  // vec2
    // constexpr unsigned int attribute2d_normal_amount = 3;    // vec3

    //Vertex array object abstraction, should be fairly simple, only used with OpenGL 3.3 core
    #ifdef USE_VAO
        struct VAO
        {
            GLuint m_id = empty_id;

            VAO() = default;
            ~VAO();

            // explicit init, as we dont want to call generate buffers in default constructor
            // calling code should check the validity of `m_id` afterwards!
            void init();

            void bind() const;
            void unbind() const; //TODO refactor? unbinds are an OpenGL anti-pattern
        };
    #endif

    struct AttributeConfig
    {
        // only position is not optional -> should be always larger than 0
        unsigned int pos_amount = 0;
        unsigned int texcoord_amount = 0;
        unsigned int normal_amount = 0;

        AttributeConfig() = default;

        constexpr AttributeConfig(unsigned int pos_amount, unsigned int texcoord_amount, unsigned int normal_amount)
                                    : pos_amount(pos_amount), texcoord_amount(texcoord_amount), normal_amount(normal_amount) {}
        
        unsigned int sum() const;
    };

    //Vertex buffer object abstraction, should be used mainly for mesh data - currently only supports GL_STATIC_DRAW
    //  consists of (in this order) - vertex positions, vertex texture coordinates (optional), vertex normals (optional)
    //  uses VAO when `USE_VAO` macro is defined.
    struct VBO
    {
        GLuint m_id = empty_id;

        // VAO is located here as we dont have any Mesh struct yet
        // Ideally we would want VAO to be outside of VBO and optionally bound by Mesh instead of VBO through VBO::bind
        #ifdef USE_VAO
            VAO m_vao;
        #endif
        //IDEA if we use vao then we probably dont need the following attributes

    private:
        AttributeConfig m_attr_config; // config specifying sizes of each of the vertex attributes (size is in amount of GLfloats)
        size_t m_vert_count;           // amount of vertices that this vbo holds
        size_t m_stride;               // stride in bytes (vec3 position + (optional) vec2 texcoords + (optional) vec3 normal)
        int m_texcoord_offset, m_normal_offset; // offsets into the vbo NOT in bytes, -1 means that attribute is not included

    public:
        VBO(); // default constructor for uninitialized VBO
        VBO(const GLfloat *data, size_t data_vert_count, AttributeConfig attr_config = default3DConfig);
        ~VBO();

        size_t vertexCount() const;

        VBO& operator=(VBO&& other); // this is sadly needed because of global meshes and generate functions + constructors

        void bind() const;
        void unbind() const; //TODO refactor? unbinds are an OpenGL anti-pattern, this unbind is however correct when not using VAO!

        static constexpr AttributeConfig default3DConfig = AttributeConfig{Meshes::attribute3d_pos_amount,
                                                                           Meshes::attribute3d_texcoord_amount,
                                                                           Meshes::attribute3d_normal_amount};

        static constexpr AttributeConfig default2DConfig  = AttributeConfig{Meshes::attribute2d_pos_amount, 0, 0};

    private:
        // helper methods for better readability, the code will be probably moved back to VBO::bind/unbind after VAO+Mesh refactor
        void bind_noVAO() const;
        void unbind_noVAO() const;
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
    
    struct Mesh
    {
        //TODO is m_vert_count needed as it is already stored in vbo?
        //TODO add shapes - offset + size into vectors, add draw specific shape
        unsigned int m_vert_count = 0, m_triangle_count = 0;

        std::vector<GLfloat> m_positions;
        std::vector<GLfloat> m_texcoords;
        std::vector<GLfloat> m_normals;

        VBO m_vbo;

        Mesh() = default;
        Mesh(unsigned int vert_count, std::vector<GLfloat>&& positions,
             std::vector<GLfloat>&& texcoords, std::vector<GLfloat>&& normals);

        int loadFromData(unsigned int vert_count, std::vector<GLfloat>&& positions,
                          std::vector<GLfloat>&& texcoords, std::vector<GLfloat>&& normals);
        int loadFromObj(const char *obj_file_path);

        bool upload();

        bool isUploaded() const;

        void draw() const;
    };

    Meshes::Mesh generateCubicMesh(glm::vec3 mesh_scale, glm::vec2 texture_world_size, Meshes::TexcoordStyle style);
    
    Meshes::Mesh generateQuadMesh(glm::vec2 mesh_scale, glm::vec2 texture_world_size, Meshes::TexcoordStyle style);

    struct Model
    {
        const Shaders::Program& m_shader;
        Lighting::Material m_material;

        //TODO add mesh shape after it is implemented
        const Mesh& m_mesh;

        //TODO add rotation too
        glm::vec3 m_origin_offset, m_translate, m_scale;

        Model(const Shaders::Program& shader, const Meshes::Mesh& mesh, Lighting::Material material);

        //TODO add rotation as a parameter too
        void draw(const Drawing::Camera3D& camera, const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                  float gamma, glm::vec3 pos, glm::vec3 scale = glm::vec3(1.f)) const;
        
        void drawWithColorTint(const Drawing::Camera3D& camera,
                               const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                               float gamma, glm::vec3 pos, const Color3F color_tint, glm::vec3 scale = glm::vec3(1.f)) const;
    };

    int loadObj(const char *obj_file_path, unsigned int *out_vert_count, unsigned int *out_triangle_count,
                std::vector<GLfloat>& out_positions, std::vector<GLfloat>& out_texcoords, std::vector<GLfloat>& out_normals,
                std::vector<Lighting::MaterialProps> *out_material_props);
    
    int loadMtl(const char *mtl_file_path, std::vector<Lighting::MaterialProps>& out_material_props);
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

        RayCollision() = default;                       // init no hit
        RayCollision(float travel, glm::vec3 point);    // init hit
    };

    glm::vec3 closestRayProjection(Ray ray, glm::vec3 point);

    RayCollision rayPlane(Ray ray, glm::vec3 plane_normal, glm::vec3 plane_pos);

    RayCollision rayTarget(Ray ray, glm::vec3 target_normal, glm::vec3 target_pos, float target_radius);

    RayCollision raySphere(Ray ray, glm::vec3 sphere_pos, float sphere_radius);

    RayCollision rayFlatTargets(Ray ray, const std::vector<Game::Target>& targets,
                                double frame_time, size_t *out_idx);

    RayCollision rayBallTargets(Collision::Ray ray, const std::vector<Game::Target>& ball_targets,
                                double frame_time, size_t *out_idx);
}

namespace UI
{
    // vertex struct that servers ONLY as a model of data that gets passed into UI shader from Nuklear through UI Context's VBO
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
        #ifdef USE_VAO
            Meshes::VAO m_vao;
        #endif
        GLuint m_vbo_id, m_ebo_id;

        Context(const Shaders::Program& shader, const UI::Font& font);
        ~Context();

        bool getInput(GLFWwindow* window, glm::vec2 mouse_pos, bool mouse_left_is_down,
                      unsigned int textbuffer[], size_t textbuffer_len);

        bool convert();

        bool draw(glm::vec2 screen_res, unsigned int texture_unit = 0);

        void clear();

        //GUI helper methods
        void verticalGap(float gap_height);
    
    private:
        void setupVBOAttributes() const;
        
        void disableVBOAttributes() const;
    };
}

//game.cpp
namespace Game
{
    glm::vec3 targetMiddleWallPosition(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size);
    glm::vec3 targetRandomWallPosition(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size);

    //TODO this to take double as a template parameter
    template <unsigned int factor> float targetGetScale_linearFactor(double alive_time);

    struct PosChanger
    {
        glm::vec3 m_pos;

        PosChanger(glm::vec3 init_pos) : m_pos(init_pos) {}
        virtual ~PosChanger() = default;

        virtual void updatePos(glm::vec3 size, double alive_time) {}
    };

    struct PosChanger_float : PosChanger
    {
        glm::vec3 m_dir;
        glm::vec3 m_area_pos;
        glm::vec2 m_area_size;
        float m_prev_alive_time;
        float m_speed;

        struct Params { glm::vec2 area_size; glm::vec3 area_pos_offset; float speed; };

        PosChanger_float(glm::vec3 init_pos, glm::vec3 dir, glm::vec3 spawn_area_pos, glm::vec2 spawn_area_size, Params params);
        ~PosChanger_float() = default;

        virtual void updatePos(glm::vec3 size, double alive_time) override;
    };

    enum class TargetType { target, ball };

    class Target
    {
        static float getScale_default(double time);

        PosChanger& getCurrentPosChanger();
        const PosChanger& getCurrentPosChanger() const;
    public:
        typedef float (ScaleFnPtr)(double alive_time);
        using PosChangerVariant = std::variant<PosChanger, PosChanger_float>;

        constexpr static const float size_min = 0.2f, size_max = 1.f; // in scale to target size
        constexpr static const float grow_time = 2.5f; // 2.5 seconds
        constexpr static const float flat_target_size = 0.5f;
        constexpr static const float ball_target_size = 0.35f;

        const Meshes::Model& m_model;

        Color3F m_color_tint;
        ScaleFnPtr *m_scale_fn;
        PosChangerVariant m_pos_changer;
        double m_spawn_time;

        Target(const Meshes::Model& model, double spawn_time, PosChangerVariant&& pos_changer,
               Color3F color_tint = Color3F(1.f, 1.f, 1.f), ScaleFnPtr *m_scale_fn = NULL);
        Target(const Target& other);
        ~Target() = default;

        Target& operator=(const Target& other);

        float getScale(double time) const;

        void updatePos(double current_frame_time);

        glm::vec3 getPos() const;

        void draw(Game::TargetType type, const Drawing::Camera3D& camera,
                  const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                  float gamma, double current_frame_time, glm::vec3 pos_offset = glm::vec3(0.f)) const;
    };

    struct LevelPart
    {
        typedef glm::vec3 (SpawnNextFnPtr)(Utils::RNG&, Utils::RNG&, glm::vec2);
        SpawnNextFnPtr *m_spawn_next_fn;
        Target::ScaleFnPtr *m_scale_fn;

        // monostate is for the base PosChanger type
        using PosChangerParamsVariant = std::variant<std::monostate, Game::PosChanger_float::Params>;
        PosChangerParamsVariant m_pos_changer_params;
        
        TargetType m_type;
        unsigned int m_target_amount;
        float m_spawn_rate;
        Color3F m_color;

        LevelPart(TargetType type, unsigned int target_amount, float spawn_rate,
                  SpawnNextFnPtr spawn_next_fn = Game::targetRandomWallPosition,
                  PosChangerParamsVariant pos_changer_params = std::monostate{},
                  Target::ScaleFnPtr scale_fn = NULL, Color3F color = Color3F{ 1.0, 1.0, 1.0 });

        glm::vec3 nextSpawnPos(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size) const;
        
        Target::PosChangerVariant spawnNext(Utils::RNG& width, Utils::RNG& height, Utils::RNG& angle,
                                            glm::vec3 wall_pos, glm::vec2 wall_size) const;
    };

    class Level
    {
        std::vector<unsigned int> m_target_amount_cum; // cummulative count of target amounts for level parts
        unsigned int getCummulativeCountUptoIndex(unsigned int idx) const; // returns cummulative count upto part idx (not counting it's amount)
    public:
        std::vector<LevelPart> m_level_parts;
        bool m_immediate_spawn; // spawn next target immediately if there are no targets spawned

        Level(std::vector<LevelPart>&& level_parts, bool immediate_spawn = false);
        ~Level() = default;

        const LevelPart& getPart(unsigned int idx) const;
        unsigned int getPartIdxFromSpawnedTargets(unsigned int targets_spawned) const;

        unsigned int getTargetAmount() const;
    };

    class LevelManager
    {
        std::vector<Level> m_levels;

        double m_last_spawn_time = 0.0; //IDEA maybe use negative number to indicate immediate respawn?
    public:
        unsigned int m_level_idx = 0, m_level_targets_hit = 0;

        void addLevel(Level&& level);
        const Level& getLevel(unsigned int idx) const;
        const Level& getCurrentLevel() const;
        const LevelPart* getCurrentLevelPart(unsigned int targets_alive) const;

        unsigned int getCurrentLevelTargetAmount() const;
        unsigned int getLevelAmount() const;
        unsigned int getPartialTargetAmount(unsigned int level_from, unsigned int level_amount) const;
        unsigned int getWholeTargetAmount() const;

        void prepareFirstLevel(double frame_time);
        bool handleTargetHit(double frame_time);
        unsigned int targetSpawnAmount(double frame_time, unsigned int targets_alive);

        bool levelsCompleted() const;
    };
}

//loop_data.cpp
enum class LoopRetVal { exit, ok, popTop };

struct LoopData // vtable + pointer to data itself
{
    typedef int         (InitFnPtr)         (void *data);
    typedef void        (DeinitFnPtr)       (void *data);
    typedef LoopRetVal  (LoopCallbackFnPtr) (void *data, unsigned int global_tick, double frame_time, float frame_delta);

    std::unique_ptr<unsigned char[]> m_raw_data;

    InitFnPtr *m_init_fn;
    DeinitFnPtr *m_deinit_fn;
    LoopCallbackFnPtr *m_loop_callback_fn;

    LoopData(size_t data_size, InitFnPtr *init_fn, DeinitFnPtr *deinit_fn, LoopCallbackFnPtr *loop_callback_fn);
    LoopData(LoopData&& other);
    ~LoopData();

    void* getData() const;
    bool dataInitialized() const;

    int init() const;
    void deinit() const;
    void deinitAndFree();
    LoopRetVal loopCallback(unsigned int global_tick, double frame_time, float frame_delta) const;

    template <typename T>
    static int init_template(void *data)
    {
        return reinterpret_cast<T*>(data)->init();
    }

    template <typename T>
    static void deinit_template(void *data)
    {
        // reinterpret_cast<T*>(data)->T::~T();
        // reinterpret_cast<T*>(data)->~T();
        std::destroy_at(reinterpret_cast<T*>(data));
    }

    template <typename T>
    static LoopRetVal loop_template(void *data, unsigned int global_tick, double frame_time, float frame_delta)
    {
        return reinterpret_cast<T*>(data)->loop(global_tick, frame_time, frame_delta);
    }

    template <typename T>
    static LoopData createFromType()
    {
        return LoopData(sizeof(T), init_template<T>, deinit_template<T>, loop_template<T>);
    }
};

class MainLoopStack
{
    std::vector<LoopData> m_stack;

    double m_last_frame_time = -1.f;

public:
    const LoopData* currentLoopData() const;

    LoopData* push(LoopData&& new_data);

    template <typename T>
    LoopData* pushFromTemplate()
    {
        return push(LoopData::createFromType<T>());
    }

    float getFrameDelta(double frame_time);

    void pop();

    static MainLoopStack instance;
};

//shared_gl_context.cpp
struct SharedGLContext
{
    //VBOs and Meshes
    Meshes::VBO unit_quad_pos_only;

    //Basic Textures
    Textures::Texture2D white_pixel_tex;

    //3D Framebuffer
private:
    static constexpr GLenum fbo3d_rbo_color_internalformat = GL_RGB565;
    Textures::Texture2D fbo3d_conv_tex;
    GLuint fbo3d_rbo_color;
    #ifdef USE_COMBINED_FBO_BUFFERS
        GLuint fbo3d_rbo_depth_stencil;
    #else
        GLuint fbo3d_rbo_depth; 
        GLuint fbo3d_rbo_stencil;
    #endif
    Drawing::FrameBuffer fbo3d_unconv, fbo3d_conv;
    unsigned int fbo3d_samples;
    glm::ivec2 fbo3d_unconv_size;
public:
    bool use_fbo3d, use_msaa, enable_gamma_correction;

    SharedGLContext(bool use_fbo3d, unsigned int init_width, unsigned int init_height, unsigned int fbo3d_samples, bool use_msaa, bool enable_gamma_correction);
    ~SharedGLContext();

    bool isInitialized() const;

    glm::ivec2 getFbo3DSize(bool converted) const;

    void changeFbo3DSize(unsigned int new_width, unsigned int new_height);

    bool convertFbo3D() const; // resolves fbo3d_conv from unconverted internal fbo3d
    void saveToFbo3DFromExternal(GLuint external_fbo_id); // saves data into fbo3d_conv from external fbo
    // this calls either `convertFbo3D` or `saveToFbo3DFromExternal` based on given parameters
    bool stageFbo3D(std::optional<GLuint> external_fbo_id_used);

    const Textures::Texture2D& getFbo3DTexture() const;
    const Drawing::FrameBuffer& getFbo3D(bool converted) const;

    static std::optional<SharedGLContext> instance;
};

//Game loops:
//main-test.cpp
struct TestMainLoop //TODO proper deinit of objects
{
    //Camera
    float mouse_sens, fov, camera_pitch, camera_yaw;
    Drawing::Camera3D camera;

    //VBOs
    Meshes::VBO cube_vbo;

    //UI
    Shaders::Program ui_shader;
    UI::Font font;
    UI::Context ui;

    //Shaders
    Shaders::Program tex_rect_shader, light_src_shader, light_shader;
    
    //Textures
    Textures::Texture2D fbo3d_tex, brick_texture, orb_texture;

    //RenderBuffers
    GLuint fbo3d_rbo_depth, fbo3d_rbo_stencil;

    //FrameBuffers
    Drawing::FrameBuffer fbo3d;

    //Lighting
    Lighting::DirLight sun;
    Color3F pointl_color, pointl_spec_color;
    float pointl_ambient_intensity;
    Lighting::PointLight pointl;
    glm::vec3 mat_cubes_pos;
    float movingl_x_min, movingl_x_max, movingl_move_per_sec;
    Lighting::PointLight movingl;
    Lighting::SpotLight flashlight;
    bool show_pointl, show_flashlight, movingl_pos_move;
    float gamma_coef; //TODO add into global settings

    //Materials
    Lighting::MaterialProps default_material, materials[16];

    //Misc.
    Color clear_color_3d, clear_color_2d;
    unsigned int tick;
    double last_mouse_x, last_mouse_y;

    int init();
    ~TestMainLoop();

    LoopRetVal loop(unsigned int global_tick, double frame_time, float frame_delta);
};

//main-game.cpp
struct GameMainLoop
{
    //Camera
    float mouse_sens, fov, camera_aspect_ratio, camera_pitch, camera_yaw;
    Drawing::Camera3D camera;

    //VBOs and Meshes
    Meshes::VBO cube_vbo, line_vbo;
    Meshes::Mesh turret_mesh, ball_mesh, rock_mesh, floor_mesh;
    glm::vec2 floor_size;

    //Textures and Cubemaps
    Textures::Texture2D brick_texture, brick_alt_texture, orb_texture, target_texture,
                        turret_texture, ball_texture, water_specular_map, rock_texture, wood_texture;
    glm::vec2 brick_texture_world_size, brick_alt_texture_world_size, orb_texture_world_size;
    static constexpr glm::vec2 target_texture_world_size = glm::vec2(1.f, 1.f); // 1:1 aspect ratio should be kept
    static constexpr float target_texture_dish_radius = 0.915f / 2.f; // radius of the target dish compared to the size of the full image (1.0x1.0)
    static constexpr glm::vec2 wood_texture_world_size = glm::vec2(1.75f); // 1:1 aspect ratio should be kept
    Textures::Cubemap skybox_cubemap;

    //RenderBuffers
    // GLuint fbo3d_rbo_depth, fbo3d_rbo_stencil;

    //Shaders
    Shaders::Program screen_line_shader, ui_shader, tex_rect_shader, light_src_shader, light_shader, skybox_shader;

    //Lighting
    Lighting::DirLight sun;
    Lighting::SpotLight flashlight;
    bool show_flashlight;
    static constexpr Color3F muzzle_flash_color{244.f/255.f, 208.f/255.f, 61.f/255.f};
    static constexpr Color3F muzzle_flash_specular_color{250.f/255.f, 240.f/255.f, 180.f/255.f};
    static constexpr float muzzle_flash_diffuse_coef = 0.13f, muzzle_flash_specular_coef = 0.5f;
    Lighting::PointLight muzzle_flash;
    static constexpr float muzzle_flash_duration = 0.06f; // in seconds
    double muzzle_flash_begin;

    //Materials and MaterialProps
    Lighting::MaterialProps default_material_props;
    Lighting::Material default_material, turret_material, ball_material, target_material,
                       rock_material, floor_material;

    //UI
    unsigned int textbuffer[UNICODE_TEXTBUFFER_LEN];
    size_t textbuffer_len;
    UI::Font font;
    UI::Context ui;

    //FrameBuffers
    // Drawing::FrameBuffer fbo3d;

    //Game
    static constexpr float ball_world_radius = 0.11f;
    static constexpr glm::vec3 ball_origin_offset = glm::vec3(0.f, -ball_world_radius, 0.f);
    glm::vec3 wall_size, wall_pos, wall_center;
    Meshes::VBO wall_vbo;
    Meshes::Mesh target_mesh;
    Meshes::Model target_model, ball_model, rock_model;
    std::vector<Game::Target> targets, ball_targets;
    Utils::RNG target_rng_width, target_rng_height, target_rng_dir;
    Game::LevelManager level_manager;
    double practice_time_start, practice_time_end;
    std::vector<float> pracice_times;

    //Misc.
    Color clear_color_3d, clear_color_2d;
    unsigned int tick, last_global_tick;
    double fps_calculation_interval, last_fps_calculation_time;
    unsigned int fps_calculation_counter, fps_calculated;
    glm::vec2 last_mouse_posF;
    bool last_left_mbutton, last_right_mbutton;
    int last_esc_state, last_c_state;
    float gamma_coef; //TODO add into global settings

    int init();
    ~GameMainLoop();

    LoopRetVal loop(unsigned int global_tick, double frame_time, float frame_delta);

private:
    //partial init and their repsective deinits, they are only supposed to be called during main `init` method!
    //  destructor does the job of deinits automatically, but we can't call destructor before the whole init is completed
    void initCamera();
    void deinitCamera();
    bool initVBOsAndMeshes();
    void deinitVBOsAndMeshes();
    bool initTextures();
    void deinitTextures();
    // bool initRenderBuffers();
    // void deinitRenderBuffers();
    bool initShaders();
    void deinitShaders();
    void initLighting();
    void deinitLighting();
    void initMaterials();
    void deinitMaterials();
    bool initUI();
    void deinitUI();
    // bool initFrameBuffers();
    // void deinitFrameBuffers();
    bool initGameStuff();
    void deinitGameStuff();

    //other utility methods
    unsigned int getTargetsAlive() const;
    void handleTargetHit(double current_frame_time);
    bool updateMuzzleFlashLightProps(double current_frame_time);
};

//main-menu.cpp
struct GamePauseMainLoop
{
    //Textures
    Textures::Texture2D background_tex;

    //Shaders
    Shaders::Program screen_line_shader, ui_shader, gray_tex_rect_shader;

    //UI
    unsigned int textbuffer[UNICODE_TEXTBUFFER_LEN];
    size_t textbuffer_len;
    UI::Font font;
    UI::Context ui;

    //Misc.
    Color clear_color;
    unsigned int tick, last_global_tick;
    int last_esc_state;

    int init();
    ~GamePauseMainLoop();

    LoopRetVal loop(unsigned int global_tick, double frame_time, float frame_delta);

private:
    bool initTextures();
    void deinitTextures();
    bool initShaders();
    void deinitShaders();
    bool initUI();
    void deinitUI();
};

struct GameOptionsMainLoop // also in main-menu.cpp
{
    //Parameters
    Textures::Texture2D *ref_background_tex;
    Shaders::Program *ref_ui_shader;
    Shaders::Program *ref_gray_tex_rect_shader;
    UI::Context *ref_ui;

    //Shaders

    //UI
    unsigned int textbuffer[UNICODE_TEXTBUFFER_LEN];
    size_t textbuffer_len;

    //Misc.
    Color clear_color;
    unsigned int tick, last_global_tick;
    int last_esc_state;

    void setParameters(Textures::Texture2D& background_tex, Shaders::Program& ui_shader,
                       Shaders::Program& tex_rect_shader, UI::Context& ui);

    int init();
    ~GameOptionsMainLoop();

    LoopRetVal loop(unsigned int global_tick, double frame_time, float frame_delta);

private:
    bool initUI();
    void deinitUI();
};

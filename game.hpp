#pragma once

#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

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
#include <cmath>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <random>
#include <optional>

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
#define NORMALIZE_OR_0(v) (Utils::isZero((v)) ? glm::vec3(0.f) : (v))
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
    Color3F(GLfloat r, GLfloat g, GLfloat b)
            : r(r), g(g), b(b) {}
    Color3F(GLfloat val)
            : r(val), g(val), b(val) {}
    Color3F(glm::vec3 color)
            : r(color.r), g(color.g), b(color.b) {}
    Color3F(const GLfloat rgb[])
            : r(rgb[0]), g(rgb[1]), b(rgb[2]) {}
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
        enum class AttachmentType { none, render, texture };
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

        LightProps(Color3F color, float ambient_intensity)
                    : m_ambient(), m_diffuse(color), m_specular(1.f, 1.f, 1.f)
        {
            assert(ambient_intensity >= 0.f && ambient_intensity <= 1.f);
            m_ambient = Color3F(ambient_intensity * color.r, ambient_intensity * color.g, ambient_intensity * color.b);
        }
    };

    // Lights - directional (dir vec), point (pos vec), spot (dir vec, pos vec, inner/outer cone cutoff angle)
    constexpr size_t lights_max_amount = 10; //TODO make this synchronized with light shader includes!

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
        #define SHADER_ATTR_DEFINES_FS "#define IN_ATTR varying\n#define TEXTURE2D(s,c) (texture2D((s), (c)))\n#define OUTPUT_COLOR(c) (gl_FragColor = (c))\n"
    #else
        #define SHADER_VER_LINE "#version 330 core\n"
        #define SHADER_ATTR_DEFINES_VS "#define IN_ATTR in\n#define OUT_ATTR out\n"
        #define SHADER_ATTR_DEFINES_FS "#define IN_ATTR in\n#define TEXTURE2D(s,c) (texture((s), (c)))\nout vec4 FragColor;\n#define OUTPUT_COLOR(c) (FragColor = (c))\n"
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

        Texture2D() = default;
        Texture2D(unsigned int width, unsigned int height, GLenum component_type);
        Texture2D(const char *image_path, bool generate_mipmaps = default_generate_mipmaps);
        Texture2D(const void *img_data, unsigned int width, unsigned int height,
                  bool generate_mipmaps = default_generate_mipmaps);
        Texture2D(Color3 color);
        ~Texture2D();

        void bind(unsigned int unit = 0) const;

        void changeTexture(unsigned int new_width, unsigned int new_height,
                           GLenum component_type = GL_RGBA, const void *new_data = NULL);
        void changeTextureToPixel(Color3 color);

        Drawing::FrameBuffer::Attachment asFrameBufferAttachment() const;
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
        //TODO add shapes - offset + size into vectors, add draw specific shape
        unsigned int m_vert_count = 0, m_triangle_count = 0;

        std::vector<GLfloat> m_positions;
        std::vector<GLfloat> m_texcoords;
        std::vector<GLfloat> m_normals;

        VBO m_vbo;

        Mesh() = default;

        int loadFromObj(const char *obj_file_path);

        bool upload();

        bool isUploaded() const;

        void draw() const;
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

        RayCollision() = default;
        RayCollision(float travel, glm::vec3 point);
        ~RayCollision() = default;
    };

    RayCollision rayPlane(Ray ray, glm::vec3 plane_normal, glm::vec3 plane_pos);

    RayCollision rayTarget(Ray ray, glm::vec3 target_normal, glm::vec3 target_pos, float target_radius);
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
        void horizontalGap(float gap_height);
    
    private:
        void setupVBOAttributes() const;
        
        void disableVBOAttributes() const;
    };
}

//game.cpp
namespace Game
{
    class Target
    {
    public:
        constexpr static const float size_min = 0.1f, size_max = 0.5f;
        constexpr static const float grow_time = 2.5f; // 2.5 seconds

        const Meshes::VBO& m_vbo;
        const Lighting::Material& m_material;

        glm::vec3 m_pos;
        double m_spawn_time;

        Target(const Meshes::VBO& vbo, const Lighting::Material& material, glm::vec3 pos, double spawn_time);
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

//shared_gl_context.cpp
struct SharedGLContext
{
    //VBOs and Meshes
    Meshes::VBO unit_quad_pos_only;

    //Basic Textures
    Textures::Texture2D white_pixel_tex;

    //3D Framebuffer
private:
    Textures::Texture2D fbo3d_tex;
    #ifdef USE_COMBINED_FBO_BUFFERS
        GLuint fbo3d_rbo_depth_stencil;
    #else
        GLuint fbo3d_rbo_depth; 
        GLuint fbo3d_rbo_stencil;
    #endif
    Drawing::FrameBuffer fbo3d;
public:
    bool use_fbo3d;

    SharedGLContext(bool use_fbo3d, unsigned int init_width, unsigned int init_height);
    ~SharedGLContext();

    bool isInitialized() const;

    glm::ivec2 getFbo3DSize() const;

    void changeFbo3DSize(unsigned int new_width, unsigned int new_height);

    const Textures::Texture2D& getFbo3DTexture() const;
    const Drawing::FrameBuffer& getFbo3D() const;

    static std::optional<SharedGLContext> instance;
};

//Game loops
enum class LoopRetVal { exit, success };

/*struct LoopData
{
    typedef int InitFnPtr();
    typedef LoopRetVal LoopFnPtr(void*);

    void *data;
    InitFnPtr *initFn;
    LoopFnPtr *loopFn;
};*/

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
    float light_src_size;
    Lighting::DirLight sun;
    Color3F pointl_color, pointl_spec_color;
    float pointl_ambient_intensity;
    Lighting::PointLight pointl;
    glm::vec3 mat_cubes_pos;
    float movingl_x_min, movingl_x_max, movingl_move_per_sec;
    Lighting::PointLight movingl;
    Lighting::SpotLight flashlight;
    bool show_pointl, show_flashlight, movingl_pos_move;

    //Materials
    Lighting::MaterialProps default_material, materials[16];

    //Misc.
    Color clear_color_3d, clear_color_2d;
    unsigned int tick;
    double last_frame_time;
    double last_mouse_x, last_mouse_y;

    int init();
    ~TestMainLoop();

    LoopRetVal loop();
};

//main-game.cpp
struct GameMainLoop
{
    //Camera
    float mouse_sens, fov, camera_aspect_ratio, camera_pitch, camera_yaw;
    Drawing::Camera3D camera;

    //VBOs and Meshes
    Meshes::VBO cube_vbo, line_vbo;
    Meshes::Mesh turret_mesh, ball_mesh;

    //Textures
    Textures::Texture2D brick_texture, brick_alt_texture, orb_texture, target_texture,
                        turret_texture, ball_texture, water_specular_map;
    glm::vec2 brick_texture_world_size, brick_alt_texture_world_size, orb_texture_world_size, target_texture_world_size;
    float target_texture_dish_radius; // radius of the target dish compared to the size of the full image (1.0x1.0)

    //RenderBuffers
    // GLuint fbo3d_rbo_depth, fbo3d_rbo_stencil;

    //Shaders
    Shaders::Program screen_line_shader, ui_shader, tex_rect_shader, light_src_shader, light_shader;

    //Lighting
    float light_src_size;
    Lighting::DirLight sun;
    Lighting::SpotLight flashlight;
    bool show_flashlight;

    //Materials and MaterialProps
    Lighting::MaterialProps default_material_props;
    Lighting::Material default_material, ball_material, target_material;

    //UI
    unsigned int textbuffer[UNICODE_TEXTBUFFER_LEN];
    size_t textbuffer_len;
    UI::Font font;
    UI::Context ui;

    //FrameBuffers
    // Drawing::FrameBuffer fbo3d;

    //Game
    glm::vec3 wall_size, wall_pos, target_pos_offset;
    Meshes::VBO wall_vbo, target_vbo;
    std::vector<Game::Target> targets;
    Utils::RNG target_rng_width, target_rng_height;

    //Level variables
    static constexpr double level_spawn_rate_init = 0.6f; // target per second
    static constexpr double level_spawn_rate_mult = 1.35f;
    static constexpr size_t level_amount_init = 8;
    static constexpr size_t level_amount_inc = 4;

    double target_last_spawn_time, level_spawn_rate;
    unsigned int level, level_targets_hit;

    //Misc.
    Color clear_color_3d, clear_color_2d;
    unsigned int tick;
    float frame_delta;
    double last_frame_time, fps_calculation_interval, last_fps_calculation_time;
    unsigned int fps_calculation_counter, fps_calculated;
    double last_mouse_x, last_mouse_y;
    bool last_left_mbutton;

    int init();
    ~GameMainLoop();

    LoopRetVal loop();

    //TODO global mouse button manager for all main loops
    static bool left_mbutton_state;
    static void mouseButtonsCallback(GLFWwindow *window, int button, int action, int mods);

private:
    //partial init and their repsective deinits, they are only supposed to be called during main `init` method!
    //destructor does the job of deinits automatically, but we can't call destructor before the whole init is completed
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
};

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/gtc/type_ptr.hpp"

#include <array>

#define ERR_MSG_MAX_LEN 1024

#define UNIFORM_MATERIAL_NAME "material"
#define UNIFORM_MATERIAL_AMBIENT "ambient"
#define UNIFORM_MATERIAL_DIFFUSE "diffuse"
#define UNIFORM_MATERIAL_SPECULAR "specular"
#define UNIFORM_MATERIAL_SHININESS "shininess"

#define UNIFORM_LIGHTSRC_NAME "lightSrc"
#define UNIFORM_LIGHTSRC_POSITION "pos"
#define UNIFORM_LIGHTSRC_AMBIENT "ambient"
#define UNIFORM_LIGHTSRC_DIFFUSE "diffuse"
#define UNIFORM_LIGHTSRC_SPECULAR "specular"


Shaders::Program::Program(GLuint vs_id, GLuint fs_id)
                    : m_id(Shaders::programLink(vs_id, fs_id))
{}

Shaders::Program::Program(const char *vs_path, const char *fs_path)
                    : m_id(Shaders::empty_id)
{
    // constructs shader program based on source code of vertex and fragment shader located at given paths,
    // caller should always check whether constructor failed -> m_id == Shaders::empty_id
    char *vs_source = Utils::getTextFileAsString(vs_path),
         *fs_source = Utils::getTextFileAsString(fs_path);
    
    if (!vs_source || !fs_source)
    {
        fprintf(stderr, "Failed to load vertex or fragment shader source code when constructing shader program!\n");
        delete[] vs_source;
        delete[] fs_source;

        return;
    }

    GLuint vs_id = Shaders::fromString(GL_VERTEX_SHADER, vs_source),
           fs_id = Shaders::fromString(GL_FRAGMENT_SHADER, fs_source);
    if (!vs_id || !fs_id)
    {
        fprintf(stderr, "Failed to initialize vertex and fragment shaders when constructing shader program!\n");
        // fprintf(stderr, "vs_id == 0: %d\n", vs_id == 0);
        // fprintf(stderr, "fs_id == 0: %d\n", fs_id == 0);
        glDeleteShader(vs_id);
        glDeleteShader(fs_id);
        delete[] vs_source;
        delete[] fs_source;
        
        return;
    }
    
    m_id = Shaders::programLink(vs_id, fs_id);
    if (m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Shader program failed to link!\n");
    }

    glDeleteShader(vs_id);
    glDeleteShader(fs_id);
    delete[] vs_source;
    delete[] fs_source;
}

Shaders::Program::~Program()
{
    glDeleteProgram(m_id);
}

void Shaders::Program::use() const
{
    glUseProgram(m_id);
}

//USELESS probably better to use glm::vec4 version
/*void Shaders::Program::set(const char *uniform_name, std::array<float, 4> floats) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    glUniform4f(location, floats[0], floats[1], floats[2], floats[3]);
}*/

void Shaders::Program::set(const char *uniform_name, Color3F color) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform3f(location, color.r, color.g, color.b);
}

void Shaders::Program::set(const char *uniform_name, glm::vec3 vec) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform3f(location, vec.x, vec.y, vec.z);
}

void Shaders::Program::set(const char *uniform_name, glm::vec4 vec) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

void Shaders::Program::set(const char *uniform_name, GLint value) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform1i(location, value);
}

void Shaders::Program::set(const char *uniform_name, GLfloat value) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform1f(location, value);
}

void Shaders::Program::set(const char *uniform_name, const glm::mat3& matrix) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shaders::Program::set(const char *uniform_name, const glm::mat4& matrix) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shaders::Program::setMaterial(const Material& material) const
{
    //IDEA could use constexpr and templates
    //(see https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time)
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_AMBIENT, material.m_ambient);
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_DIFFUSE, material.m_diffuse);
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_SPECULAR, material.m_specular);
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_SHININESS, material.m_shininess);
}

void Shaders::Program::setLightSrc(const LightSrc& light_src) const
{
    //IDEA could use constexpr and templates
    //(see https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time)
    set(UNIFORM_LIGHTSRC_NAME "." UNIFORM_LIGHTSRC_POSITION, light_src.m_pos);
    set(UNIFORM_LIGHTSRC_NAME "." UNIFORM_LIGHTSRC_AMBIENT, light_src.m_ambient);
    set(UNIFORM_LIGHTSRC_NAME "." UNIFORM_LIGHTSRC_DIFFUSE, light_src.m_diffuse);
    set(UNIFORM_LIGHTSRC_NAME "." UNIFORM_LIGHTSRC_SPECULAR, light_src.m_specular);
}

GLuint Shaders::fromString(GLenum type, const char *str)
{
    unsigned int id = glCreateShader(type);
    if (!id)
    {
        printf("Failed to create shader(type 0x%x)!\n", type);

        return 0;
    }

    glShaderSource(id, 1, &str, NULL);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar msg[ERR_MSG_MAX_LEN + 1];
        glGetShaderInfoLog(id, ERR_MSG_MAX_LEN + 1, NULL, msg);
        msg[ERR_MSG_MAX_LEN] = '\0'; //just to be sure
        printf("Failed to compile shader(type 0x%x), error msg: '%s'\n", type, (char*)&msg);

        glDeleteShader(id);
        return 0;
    }

    return id;
}

GLuint Shaders::programLink(GLuint vs, GLuint fs)
{
    GLuint id = glCreateProgram();
    if (!id)
    {
        printf("Failed to create shader program!\n");

        return Shaders::empty_id;
    }

    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar msg[ERR_MSG_MAX_LEN + 1];
        glGetProgramInfoLog(id, ERR_MSG_MAX_LEN + 1, NULL, msg);
        msg[ERR_MSG_MAX_LEN] = '\0'; //just to be sure
        printf("Failed to link shader program, error msg: '%s'\n", (char*)&msg);

        glDeleteProgram(id);
        return Shaders::empty_id;
    }

    assert(id != Shaders::empty_id);
    return id;
}

void Shaders::setupVertexAttribute_float(GLuint location, size_t count, size_t offset, size_t stride)
{
    glVertexAttribPointer(location, count, GL_FLOAT, false, stride, (void*)(offset * sizeof(GLfloat)));
    glEnableVertexAttribArray(location);
}

void Shaders::disableVertexAttribute(GLuint location)
{
    glDisableVertexAttribArray(location);
}

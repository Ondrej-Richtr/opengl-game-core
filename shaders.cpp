#include "game.hpp"

#include "glm/gtc/type_ptr.hpp"

#define ERR_MSG_MAX_LEN 1024


Shaders::IncludeDefine::IncludeDefine(const char *name, const char *value)
                            : m_name(name), m_value(value)
{
    assert(m_name != NULL);
}

Shaders::ShaderInclude::ShaderInclude(const char *str)
                            : is_define(false), str(str) { assert(str != NULL); }

Shaders::ShaderInclude::ShaderInclude(IncludeDefine &&define)
                            : is_define(true), define(define) {}

Shaders::Program::Program(GLuint vs_id, GLuint fs_id)
                    : m_id(Shaders::programLink(vs_id, fs_id))
{}

Shaders::Program::Program(const char *vs_path, const char *fs_path,
                          const std::vector<ShaderInclude>& vs_includes, const std::vector<ShaderInclude>& fs_includes)
                    : m_id(empty_id)
{
    // constructs shader program based on source code of vertex and fragment shader located at given paths,
    // caller should always check whether constructor failed -> m_id == empty_id
    
    std::unique_ptr<char[]> vs_source = Utils::getTextFileAsString(vs_path, NULL),
                            fs_source = Utils::getTextFileAsString(fs_path, NULL);
    if (!vs_source || !fs_source)
    {
        fprintf(stderr, "Failed to load vertex or fragment shader source code when constructing shader program!\n");
        return;
    }

    GLuint vs_id = Shaders::fromStringWithIncludeSystem(GL_VERTEX_SHADER, vs_source.get(), vs_includes),
           fs_id = Shaders::fromStringWithIncludeSystem(GL_FRAGMENT_SHADER, fs_source.get(), fs_includes);
    if (vs_id == empty_id || fs_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize vertex and fragment shaders when constructing shader program!\n");
        glDeleteShader(vs_id);
        glDeleteShader(fs_id);
        
        return;
    }
    
    m_id = Shaders::programLink(vs_id, fs_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Shader program failed to link!\n");
    }

    // we dont need those partial shaders either way
    glDeleteShader(vs_id);
    glDeleteShader(fs_id);
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

void Shaders::Program::set(const char *uniform_name, ColorF color) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform4f(location, color.r, color.g, color.b, color.a);
}

void Shaders::Program::set(const char *uniform_name, Color3F color) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform3f(location, color.r, color.g, color.b);
}

void Shaders::Program::set(const char *uniform_name, glm::vec2 vec) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform2f(location, vec.x, vec.y);
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
    //printf("setting uniform: '%s' with int value %d\n", uniform_name, value);
    assert(location >= 0); // wrong uniform name (or type)!
    glUniform1i(location, value);
}

void Shaders::Program::set(const char *uniform_name, GLfloat value) const
{
    // USE THIS ONLY IF THIS SHADER PROGRAM IS ALREADY IN USE (e.g. use method was called beforehand)
    int location = glGetUniformLocation(m_id, uniform_name);
    if (location < 0)
    {
        fprintf(stderr, "[WARNING] Failed to find shared program location for uniform: '%s'!\n", uniform_name);
    }
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

void Shaders::Program::bindTexture(const char *sampler2d_name, const Textures::Texture2D& texture, unsigned int unit) const
{
    texture.bind(unit);
    set(sampler2d_name, static_cast<GLint>(unit));
}

void Shaders::Program::bindCubemap(const char *samplerCube_name, const Textures::Cubemap& cubemap, unsigned int unit) const
{
    cubemap.bind(unit);
    set(samplerCube_name, static_cast<GLint>(unit));
}

void Shaders::Program::bindDiffuseMap(const Textures::Texture2D& diffuse_map, int map_bind_offset) const
{
    bindTexture(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_DIFFUSE_MAP, diffuse_map,
                UNIFORM_MATERIAL_DIFFUSE_MAP_UNIT + map_bind_offset);
}

void Shaders::Program::bindSpecularMap(const Textures::Texture2D& specular_map, int map_bind_offset) const
{
    bindTexture(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_SPECULAR_MAP, specular_map,
                UNIFORM_MATERIAL_SPECULAR_MAP_UNIT + map_bind_offset);
}

void Shaders::Program::setMaterialProps(const Lighting::MaterialProps& material_props) const
{
    //IDEA could use constexpr and templates
    //(see https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time)
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_AMBIENT, material_props.m_ambient);
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_DIFFUSE, material_props.m_diffuse);
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_SPECULAR, material_props.m_specular);
    set(UNIFORM_MATERIAL_NAME "." UNIFORM_MATERIAL_SHININESS, material_props.m_shininess);
}

void Shaders::Program::setMaterial(const Lighting::Material& material, int map_bind_offset) const
{
    //props
    setMaterialProps(material.m_props);

    //maps
    bindDiffuseMap(material.m_diffuse_map);
    bindSpecularMap(material.m_specular_map);
}

bool Shaders::Program::setLight(const char *uniform_name, const Lighting::Light& light, int idx) const
{
    assert(uniform_name != NULL);

    return light.bindToShader(uniform_name, *this, idx);
}

int Shaders::Program::setLights(const char *uniform_array_name, const char *uniform_arrray_size_name,
                                 const std::vector<std::reference_wrapper<const Lighting::Light>>& lights) const
{
    int success_count = 0;

    for (size_t i = 0; i < lights.size() && success_count < Lighting::lights_max_amount; ++i)
    {
        if (setLight(uniform_array_name, lights[i], success_count))
        {
            ++success_count;
        }
    }
    
    set(uniform_arrray_size_name, success_count);

    return success_count;
}

static void setDefaultAttributeLocations(GLuint program_id)
{
    assert(program_id != empty_id);
    assert(!Utils::checkForGLError());

    glBindAttribLocation(program_id, Shaders::attribute_position_pos, ATTRIBUTE_DEFAULT_NAME_POS);
    glBindAttribLocation(program_id, Shaders::attribute_position_texcoords, ATTRIBUTE_DEFAULT_NAME_TEXCOORDS);
    glBindAttribLocation(program_id, Shaders::attribute_position_normals, ATTRIBUTE_DEFAULT_NAME_NORMALS);
    glBindAttribLocation(program_id, Shaders::attribute_position_color, ATTRIBUTE_DEFAULT_NAME_COLOR);
}

GLuint Shaders::fromString(GLenum type, const char *src)
{
    unsigned int id = glCreateShader(type);
    if (id == empty_id)
    {
        fprintf(stderr, "Failed to create shader(type 0x%x)!\n", type);

        return empty_id;
    }

    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        //TODO print the info log only when building for debug
        GLchar msg[ERR_MSG_MAX_LEN + 1];
        glGetShaderInfoLog(id, ERR_MSG_MAX_LEN + 1, NULL, msg);
        msg[ERR_MSG_MAX_LEN] = '\0'; //just to be sure
        fprintf(stderr, "Failed to compile shader(type 0x%x), error msg: '%s'\n", type, (char*)&msg);

        glDeleteShader(id);
        return empty_id;
    }

    return id;
}

//IDEA this could get replaced by stb_include.h (https://github.com/nothings/stb/blob/master/stb_include.h)
GLuint Shaders::fromStringWithIncludeSystem(GLenum type, const char *str, const std::vector<ShaderInclude>& includes)
{
    //consturcts the OpenGL shader of given type with `str` source contents and given includes
    // the order is: version related macros, includes, source
    // return OpenGL id of the constructed shader or `empty_id` if error
    assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);
    bool is_fragment_shader = type == GL_FRAGMENT_SHADER;

    //source array of up to 3 strings that will make up the complete source of the shader, consists of:
    // - SHADER_VER_INCLUDE_LINES_XX string (XX depends on which type of shader is being constructed),
    // - given includes concatenated into one string, 
    // - and source string itself loaded from either .vs or .fs file.
    const char *source_array[1 + 1 + 1] = { is_fragment_shader ? SHADER_VER_INCLUDE_LINES_FS :
                                                                 SHADER_VER_INCLUDE_LINES_VS, NULL };
    GLsizei source_array_len = 1; // start at 1 as we already included the `SHADER_VER_INCLUDE_LINES_XX`

    char includes_buffer[Shaders::IncludeDefine::include_buffer_capacity + 1] = { 0 };
    size_t includes_buffer_size = 0;

    for (size_t i = 0; i < includes.size(); ++i)
    {
        const size_t free_capacity = Shaders::IncludeDefine::include_buffer_capacity - includes_buffer_size;
        if (free_capacity == 0)
        {
            fprintf(stderr, "Error when copying shader includes into include buffer - buffer is out of memory(%zu)!\n",
                            Shaders::IncludeDefine::include_buffer_capacity);
            return empty_id;
        }

        size_t written = 0;
        const ShaderInclude& include = includes[i];
        if (include.is_define)
        {
            assert(include.define.m_name != NULL);

            int write_ret = -1;
            if (include.define.m_value != NULL)
            {
                write_ret = snprintf(includes_buffer + includes_buffer_size, free_capacity + 1, // +1 as snprintf counts the term. char.
                                    "#define %s %s\n", include.define.m_name, include.define.m_value);
            }
            else
            {
                write_ret = snprintf(includes_buffer + includes_buffer_size, free_capacity + 1, // +1 as snprintf counts the term. char.
                                    "#define %s\n", include.define.m_name);
            }

            if (write_ret < 0)
            {
                fprintf(stderr, "Error when formating shader define line with name: '%s' and value: '%s'\n",
                        include.define.m_name, include.define.m_value);
                return empty_id;
            }
            written = static_cast<size_t>(write_ret);
        }
        else
        {
            assert(include.str != NULL);
            // not using strncpy here as that does not tell us if the copy was truncated or not
            int write_ret = snprintf(includes_buffer + includes_buffer_size, free_capacity + 1, // +1 as snprintf counts the term. char.
                                     "%s\n", include.str);
            
            if (write_ret < 0)
            {
                fprintf(stderr, "Error when copying string include line: '%s'\n", include.str);
                return empty_id;
            }
            written = static_cast<size_t>(write_ret);
        }

        if (written > free_capacity)
        {
            fprintf(stderr, "Error when copying shader include line, the buffer has not enough space."
                            " Needed size was: %zu, buffer capacity left: %zu\n",
                            written, free_capacity);
            return empty_id;
        }

        includes_buffer_size += written;
        assert(includes_buffer_size <= Shaders::IncludeDefine::include_buffer_capacity);
    }
    includes_buffer[includes_buffer_size] = '\0'; // term. char. just in case
    
    //add the include_buffer pointer to source strings
    if (includes_buffer_size > 0) source_array[source_array_len++] = includes_buffer;

    //finally also add the given file source string
    source_array[source_array_len++] = str;

    //create the shader OpenGL object
    unsigned int id = glCreateShader(type);
    if (id == empty_id)
    {
        fprintf(stderr, "Failed to create shader(type 0x%x)!\n", type);
        return empty_id;
    }

    //compile it
    glShaderSource(id, source_array_len, source_array, NULL);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        //TODO print the info log only when building for debug
        GLchar msg[ERR_MSG_MAX_LEN + 1];
        glGetShaderInfoLog(id, ERR_MSG_MAX_LEN + 1, NULL, msg);
        msg[ERR_MSG_MAX_LEN] = '\0'; //just to be sure
        fprintf(stderr, "Failed to compile shader(type 0x%x), error msg:\n'%s'\n", type, (char*)&msg);

        glDeleteShader(id);
        return empty_id;
    }

    return id;
}

GLuint Shaders::programLink(GLuint vs, GLuint fs)
{
    GLuint id = glCreateProgram();
    if (id == empty_id)
    {
        fprintf(stderr, "Failed to create shader program!\n");

        return empty_id;
    }

    setDefaultAttributeLocations(id);

    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    
    if (!success) {
        //TODO print the info log only when building for debug
        GLchar msg[ERR_MSG_MAX_LEN + 1];
        glGetProgramInfoLog(id, ERR_MSG_MAX_LEN + 1, NULL, msg);
        msg[ERR_MSG_MAX_LEN] = '\0'; //just to be sure
        fprintf(stderr, "Failed to link shader program, error msg:\n'%s'\n", (char*)&msg);

        glDeleteProgram(id);
        return empty_id;
    }

    assert(id != empty_id);
    return id;
}

void Shaders::setupVertexAttribute_float(GLuint location, size_t count, size_t offset, size_t stride, bool offset_in_bytes)
{
    size_t byte_offset = offset_in_bytes ? offset : offset * sizeof(GLfloat);
    glVertexAttribPointer(location, count, GL_FLOAT, false, stride, reinterpret_cast<void*>(byte_offset));
    glEnableVertexAttribArray(location);
}

void Shaders::setupVertexAttribute_ubyte(GLuint location, size_t count, size_t offset, size_t stride, bool offset_in_bytes)
{
    //NOTE: normalization is set to TRUE!
    size_t byte_offset = offset_in_bytes ? offset : offset * sizeof(GLubyte);
    glVertexAttribPointer(location, count, GL_UNSIGNED_BYTE, true, stride, reinterpret_cast<void*>(byte_offset));
    glEnableVertexAttribArray(location);
}

void Shaders::disableVertexAttribute(GLuint location)
{
    glDisableVertexAttribArray(location);
}

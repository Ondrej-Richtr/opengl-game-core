#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


Meshes::VBO::VBO(GLfloat *data, size_t data_vert_count, bool texcoords, bool normals)
                    : m_id(Meshes::empty_id),
                      m_vert_count(data_vert_count), m_stride(0),
                      m_texcoord_offset(-1), m_normal_offset(-1)
{
    assert(data_vert_count > 0);

    //offsets
    int size = Meshes::attribute_verts_amount; // vertex position is always present

    if (texcoords) 
    {
        m_texcoord_offset = size;
        size += Meshes::attribute_texcoord_amount;
    }

    if (normals) 
    {
        m_normal_offset = size;
        size += Meshes::attribute_normal_amount;
    }

    //stride is in bytes -> amount of floats included * float size 
    m_stride = size * sizeof(GLfloat);

    //creating the OpenGL buffer
    glGenBuffers(1, &m_id);

    glBindBuffer(GL_ARRAY_BUFFER, m_id);             // bind the created buffer

    // since the given array is tightly packed the whole data size is just vertex count * stride
    size_t data_size = data_vert_count * m_stride;
    glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

    if (Utils::checkForGLError())
    {
        fprintf(stderr, "Error occurred when creating VBO.\n");
        glDeleteBuffers(1, &m_id);
        m_id = Meshes::empty_id;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, Meshes::empty_id); // unbind the buffer afterwards
}

Meshes::VBO::~VBO()
{
    glDeleteBuffers(1, &m_id);
}

void Meshes::VBO::bind() const
{
    assert(m_id != Meshes::empty_id);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_id);

    //vertex position
    Shaders::setupVertexAttribute_float(Shaders::attribute_position_verts, Meshes::attribute_verts_amount,
                                        0, m_stride); // vertex position offset is always 0

    //vertex texcoords (if present)
    if (m_texcoord_offset >= 0)
    {
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_texcoords, Meshes::attribute_texcoord_amount,
                                            m_texcoord_offset, m_stride);
    }

    //vertex normal (if present)
    if (m_normal_offset >= 0)
    {
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_normals, Meshes::attribute_normal_amount,
                                            m_normal_offset, m_stride);
    }
}

void Meshes::VBO::unbind() const
{
    // assumes that VBO was already bound!

    //vertex position
    Shaders::disableVertexAttribute(Shaders::attribute_position_verts);

    //vertex texcoords (if present)
    if (m_texcoord_offset >= 0) Shaders::disableVertexAttribute(Shaders::attribute_position_texcoords);

    //vertex normal (if present)
    if (m_normal_offset >= 0) Shaders::disableVertexAttribute(Shaders::attribute_position_normals);

    glBindBuffer(GL_ARRAY_BUFFER, Meshes::empty_id);
}

Meshes::VBO Meshes::generateCubicVBO(glm::vec3 mesh_scale, glm::vec2 texture_world_size,
                                     Meshes::TexcoordStyle style, bool normals)
{
    // generates cubic vertex data with given size mesh_scale and uploads them into vbo,
    // optionaly also generates texcoords when style is not Meshes::TexcoordStyle::none,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating,
    // optionaly also generates normals,
    // returns vbo with corresponding data uploaded
    assert(mesh_scale.x > 0.f && mesh_scale.y > 0.f && mesh_scale.z > 0.f); // probably useless, we could have zero sized cubes

    GLfloat half_x = mesh_scale.x / 2.f,
            half_y = mesh_scale.y / 2.f,
            half_z = mesh_scale.z / 2.f;
    
    // U is horizontal  -> only in x and z direction
    // V is vertical    -> only in y and z direction
    GLfloat u_min_x = 0.f, u_max_x = 1.f,
            u_min_z = 0.f, u_max_z = 1.f,
            v_min_y = 0.f, v_max_y = 1.f,
            v_min_z = 0.f, v_max_z = 1.f;
    if (style == Meshes::TexcoordStyle::repeat)
    {
        assert(texture_world_size.x > 0.f && texture_world_size.y > 0.f); // avoid divide by zero
        
        // texture_world_size.x -> width of the texture in world coordinates
        // texture_world_size.y -> height of the texture in world coordinates
        u_max_x = mesh_scale.x / texture_world_size.x;
        u_max_z = mesh_scale.z / texture_world_size.x;

        v_max_y = mesh_scale.y / texture_world_size.y;
        v_max_z = mesh_scale.z / texture_world_size.y;
    }

    // printf("min/max u_x: %f|%f, u_z: %f|%f\n", u_min_x, u_max_x, u_min_z, u_max_z);
    // printf("min/max v_y: %f|%f, v_z: %f|%f\n", v_min_y, v_max_y, v_min_z, v_max_z);

    // include texcoords when style is either of those
    bool texcoords = (style == Meshes::TexcoordStyle::stretch) || (style == Meshes::TexcoordStyle::repeat);
    
    // clockwise vertex order
    const GLfloat whole_data[] =
    {
        // Position                     // Texcoords        // Normal
        //+x face                       +u = -z, +v = +y
         half_x,  half_y,  half_z,      u_min_z, v_max_y,    1.f,  0.f,  0.f,
         half_x,  half_y, -half_z,      u_max_z, v_max_y,    1.f,  0.f,  0.f,
         half_x, -half_y, -half_z,      u_max_z, v_min_y,    1.f,  0.f,  0.f,

         half_x, -half_y, -half_z,      u_max_z, v_min_y,    1.f,  0.f,  0.f,
         half_x, -half_y,  half_z,      u_min_z, v_min_y,    1.f,  0.f,  0.f,
         half_x,  half_y,  half_z,      u_min_z, v_max_y,    1.f,  0.f,  0.f,
        //-x face                       +u = z, +v = +y
        -half_x,  half_y,  half_z,      u_max_z, v_max_y,   -1.f,  0.f,  0.f,
        -half_x, -half_y, -half_z,      u_min_z, v_min_y,   -1.f,  0.f,  0.f,
        -half_x,  half_y, -half_z,      u_min_z, v_max_y,   -1.f,  0.f,  0.f,

        -half_x, -half_y, -half_z,      u_min_z, v_min_y,   -1.f,  0.f,  0.f,
        -half_x,  half_y,  half_z,      u_max_z, v_max_y,   -1.f,  0.f,  0.f,
        -half_x, -half_y,  half_z,      u_max_z, v_min_y,   -1.f,  0.f,  0.f,
        //+y face                       +u = +x, +v = -z
         half_x,  half_y,  half_z,      u_max_x, v_min_z,    0.f,  1.f,  0.f,
        -half_x,  half_y, -half_z,      u_min_x, v_max_z,    0.f,  1.f,  0.f,
         half_x,  half_y, -half_z,      u_max_x, v_max_z,    0.f,  1.f,  0.f,
        
        -half_x,  half_y, -half_z,      u_min_x, v_max_z,    0.f,  1.f,  0.f,
         half_x,  half_y,  half_z,      u_max_x, v_min_z,    0.f,  1.f,  0.f,
        -half_x,  half_y,  half_z,      u_min_x, v_min_z,    0.f,  1.f,  0.f,
        //-y face                       +u = +x, +v = +z
         half_x, -half_y,  half_z,      u_max_x, v_min_z,    0.f, -1.f,  0.f,
         half_x, -half_y, -half_z,      u_max_x, v_max_z,    0.f, -1.f,  0.f,
        -half_x, -half_y, -half_z,      u_min_x, v_max_z,    0.f, -1.f,  0.f,

        -half_x, -half_y, -half_z,      u_min_x, v_max_z,    0.f, -1.f,  0.f,
        -half_x, -half_y,  half_z,      u_min_x, v_min_z,    0.f, -1.f,  0.f,
         half_x, -half_y,  half_z,      u_max_x, v_min_z,    0.f, -1.f,  0.f,
        //+z face                       +u = +x, +v = +y
         half_x,  half_y,  half_z,      u_max_x, v_max_y,    0.f,  0.f,  1.f,
         half_x, -half_y,  half_z,      u_max_x, v_min_y,    0.f,  0.f,  1.f,
        -half_x, -half_y,  half_z,      u_min_x, v_min_y,    0.f,  0.f,  1.f,
        
        -half_x, -half_y,  half_z,      u_min_x, v_min_y,    0.f,  0.f,  1.f,
        -half_x,  half_y,  half_z,      u_min_x, v_max_y,    0.f,  0.f,  1.f,
         half_x,  half_y,  half_z,      u_max_x, v_max_y,    0.f,  0.f,  1.f,
        //-z face                       +u = -x, +v = +y
         half_x,  half_y, -half_z,      u_min_x, v_max_y,    0.f,  0.f, -1.f,
        -half_x, -half_y, -half_z,      u_max_x, v_min_y,    0.f,  0.f, -1.f,
         half_x, -half_y, -half_z,      u_min_x, v_min_y,    0.f,  0.f, -1.f,
        
        -half_x, -half_y, -half_z,      u_max_x, v_min_y,    0.f,  0.f, -1.f,
         half_x,  half_y, -half_z,      u_min_x, v_max_y,    0.f,  0.f, -1.f,
        -half_x,  half_y, -half_z,      u_max_x, v_max_y,    0.f,  0.f, -1.f
    };

    const size_t whole_data_len = sizeof(whole_data) / sizeof(whole_data[0]);
    // 8 = 3 for position + 2 for texcoords + 3 for normal
    assert(whole_data_len % 8 == 0);
    const size_t whole_data_vert_count = whole_data_len / 8;

    size_t stride = Meshes::attribute_verts_amount; // vertex position always included
    if (style != Meshes::TexcoordStyle::none) stride += Meshes::attribute_texcoord_amount;
    if (normals) stride += Meshes::attribute_normal_amount;
    assert(stride > 0);
    assert(stride <= 8);

    GLfloat data[sizeof(whole_data)];

    GLfloat *current_whole_data = (GLfloat*)whole_data,
            *current_data = data;

    for (size_t i = 0; i < whole_data_vert_count; ++i) // cycle through each vertex in whole_data
    {
        // copy position
        memcpy(current_data, current_whole_data, Meshes::attribute_verts_amount * sizeof(GLfloat));

        // texcoord data
        if (texcoords)
        {
            // copy texcoords
            memcpy(current_data + Meshes::attribute_verts_amount,
                   current_whole_data + Meshes::attribute_verts_amount,
                   Meshes::attribute_texcoord_amount * sizeof(GLfloat));

            // copy normal
            if (normals)
            {
                memcpy(current_data + Meshes::attribute_verts_amount + Meshes::attribute_texcoord_amount,
                       current_whole_data + Meshes::attribute_verts_amount + Meshes::attribute_texcoord_amount,
                       Meshes::attribute_normal_amount * sizeof(GLfloat));
            }
        }
        // no texcoord data
        else if (normals)
        {
            // just copy normal
            memcpy(current_data + Meshes::attribute_verts_amount,
                   current_whole_data + Meshes::attribute_verts_amount,
                   Meshes::attribute_normal_amount * sizeof(GLfloat));
        }

        // advance the pointers accordingly
        current_whole_data += 8;
        current_data += stride;
    }

    return Meshes::VBO(data, whole_data_vert_count, texcoords, normals);
}
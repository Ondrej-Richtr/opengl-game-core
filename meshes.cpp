#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "tinyobj_loader_c.h"


#ifdef USE_VAO
Meshes::VAO::~VAO()
{
    // printf("VAO deleted: %d\n", m_id);

    glDeleteVertexArrays(1, &m_id);
}

void Meshes::VAO::init()
{
    assert(m_id == empty_id);
    assert(!Utils::checkForGLError());

    glGenVertexArrays(1, &m_id);

    // printf("VAO initialized: %d\n", m_id);
}

void Meshes::VAO::bind() const
{
    // printf("VAO bound: %d\n", m_id);
    glBindVertexArray(m_id);
}

void Meshes::VAO::unbind() const
{
    // printf("VAO unbound: %d\n", m_id);
    glBindVertexArray(empty_id);
}
#endif

unsigned int Meshes::AttributeConfig::sum() const
{
    return pos_amount + texcoord_amount + normal_amount;
}

Meshes::VBO::VBO() : m_id(empty_id),
                     #ifdef USE_VAO
                        m_vao(),
                     #endif
                     m_attr_config(),
                     m_vert_count(0), m_stride(0),
                     m_texcoord_offset(-1), m_normal_offset(-1) {}

Meshes::VBO::VBO(const GLfloat *data, size_t data_vert_count, AttributeConfig attr_config)
                    : m_id(empty_id),
                      #ifdef USE_VAO
                         m_vao(),
                      #endif
                     m_attr_config(attr_config),
                     m_vert_count(data_vert_count), m_stride(0),
                      m_texcoord_offset(-1), m_normal_offset(-1)
{
    assert(data_vert_count > 0);
    assert(!Utils::checkForGLError()); // assert that there were no other errors beforehand (so we can safely use Utils::checkForGLError here)

    #ifdef USE_VAO
        m_vao.init();
        if (m_vao.m_id == empty_id)
        {
            fprintf(stderr, "Error occurred when creating VAO for VBO.\n");
            return;
        }
    #endif

    //offsets
    assert(m_attr_config.pos_amount > 0);
    int size = m_attr_config.pos_amount; // vertex position is always present

    if (m_attr_config.texcoord_amount > 0)
    {
        m_texcoord_offset = size;
        size += m_attr_config.texcoord_amount;
    }

    if (m_attr_config.normal_amount > 0) 
    {
        m_normal_offset = size;
        size += m_attr_config.normal_amount;
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
        m_id = empty_id;
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, empty_id); // unbind the buffer afterwards

    #ifdef USE_VAO
        // setup VAO
        m_vao.bind();
        bind_noVAO();
        m_vao.unbind();
    #endif
}

Meshes::VBO::~VBO()
{
    glDeleteBuffers(1, &m_id);
}

size_t Meshes::VBO::vertexCount() const
{
    return m_vert_count;
}

Meshes::VBO& Meshes::VBO::operator=(Meshes::VBO&& other)
{
    assert(m_id == empty_id); // use this only on empty VBOs!

    memcpy((void*)this, &other, sizeof(Meshes::VBO));

    // set empty values
    other.m_id = empty_id;
    #ifdef USE_VAO
        // this is pretty bad solution, sadly pretty much needed in C++
        // maybe we will have to define move assignment for VAOs too which would clear other.m_vao
        other.m_vao.m_id = empty_id;
    #endif
    other.m_attr_config = Meshes::AttributeConfig(); // assign the default with all zeros (maybe pointless?)
    other.m_vert_count = 0;
    other.m_stride = 0;
    other.m_texcoord_offset = 0;
    other.m_normal_offset = 0;

    return *this;
}

void Meshes::VBO::bind() const
{
    assert(m_id != empty_id);

    #ifdef USE_VAO
        m_vao.bind();
    #else
        bind_noVAO();
    #endif
}

void Meshes::VBO::unbind() const
{
    // assumes that VBO was already bound!

    #ifdef USE_VAO
        m_vao.unbind();
    #else
        unbind_noVAO();
    #endif
}

void Meshes::VBO::bind_noVAO() const
{
    assert(m_id != empty_id);

    glBindBuffer(GL_ARRAY_BUFFER, m_id);

    //vertex position
    assert(m_attr_config.pos_amount > 0);
    Shaders::setupVertexAttribute_float(Shaders::attribute_position_pos, m_attr_config.pos_amount,
                                        0, m_stride); // vertex position offset is always 0

    //vertex texcoords (if present)
    if (m_texcoord_offset >= 0)
    {
        assert(m_attr_config.texcoord_amount > 0);
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_texcoords, m_attr_config.texcoord_amount,
                                            m_texcoord_offset, m_stride);
    }

    //vertex normal (if present)
    if (m_normal_offset >= 0)
    {
        assert(m_attr_config.normal_amount > 0);
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_normals, m_attr_config.normal_amount,
                                            m_normal_offset, m_stride);
    }
}

void Meshes::VBO::unbind_noVAO() const
{
    //vertex position
    Shaders::disableVertexAttribute(Shaders::attribute_position_pos);

    //vertex texcoords (if present)
    if (m_texcoord_offset >= 0) Shaders::disableVertexAttribute(Shaders::attribute_position_texcoords);

    //vertex normal (if present)
    if (m_normal_offset >= 0) Shaders::disableVertexAttribute(Shaders::attribute_position_normals);

    glBindBuffer(GL_ARRAY_BUFFER, empty_id);
}

template <size_t whole_data_len>
static Meshes::VBO generateVBOfromData3D(const GLfloat *whole_data, bool texcoords, bool normals)
{
    // constructs VBO with mesh data given in whole_data, can ignore texcoord or normals data from the input
    // whole_data MUST have the full usual layout in correct order!
    // also the whole_data array must be of length whole_data_len!

    // layout of whole_data must be (in this order): 3 floats for position + 2 floats for texcoords + 3 floats for normal
    static_assert(whole_data_len % 8 == 0,
                  "`generateVBOfromData` function requires whole_data with length divisible by 8! (8 == 3 for vertices + 2 for uv + 3 for normals)");
    static_assert(whole_data_len > 0, "`generateVBOfromData` function requires non-empty whole_data!"); // empty data makes no sense
    constexpr size_t vert_count = whole_data_len / 8;

    if (texcoords && normals) // simple case, dont discard anything -> whole_data is already data that we need
    {
        return Meshes::VBO(whole_data, vert_count);
    }

    GLfloat data[whole_data_len];

    Meshes::AttributeConfig attr_config{}; // begin with empty config
    attr_config.pos_amount = Meshes::attribute3d_pos_amount; // vertex position always included
    if (texcoords) attr_config.texcoord_amount = Meshes::attribute3d_texcoord_amount;
    if (normals) attr_config.normal_amount = Meshes::attribute3d_normal_amount;

    unsigned int stride = attr_config.sum();
    assert(stride > 0);
    assert(stride <= 8);

    const GLfloat *current_whole_data = whole_data;
    GLfloat       *current_data       = data;

    // this is an optimization
    const unsigned int pos_amount = attr_config.pos_amount,
                       texcoord_amount = attr_config.texcoord_amount,
                       normal_amount = attr_config.normal_amount;

    for (size_t i = 0; i < vert_count; ++i) // cycle through each vertex in whole_data
    {
        // copy position
        memcpy(current_data, current_whole_data, pos_amount * sizeof(GLfloat));

        //IDEA maybe advance current_data and current_whole_data pointers after each copy,
        // that way we might be faster and also get rid off stride and advancing by 8

        // texcoord data
        if (texcoords)
        {
            assert(attr_config.texcoord_amount > 0);

            // copy texcoords
            memcpy(current_data + pos_amount,
                   current_whole_data + pos_amount,
                   texcoord_amount * sizeof(GLfloat));

            // copy normal
            if (normals)
            {
                assert(attr_config.normal_amount > 0);

                memcpy(current_data + pos_amount + texcoord_amount,
                       current_whole_data + pos_amount + texcoord_amount,
                       normal_amount * sizeof(GLfloat));
            }
        }
        // no texcoord data
        else if (normals)
        {
            assert(attr_config.normal_amount > 0);

            // just copy normal
            memcpy(current_data + pos_amount,
                   current_whole_data + pos_amount,
                   normal_amount * sizeof(GLfloat));
        }

        // advance the pointers accordingly
        current_whole_data += 8;
        current_data += stride;
    }

    // return the VBO constructed from the partial data
    return Meshes::VBO(data, vert_count, attr_config);
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

    // include texcoords when style given a Texcoord style
    bool texcoords = (style != Meshes::TexcoordStyle::none);
    
    // counter-clockwise vertex winding order
    const GLfloat whole_data[] =
    {
        // Position                     // Texcoords        // Normal
        //+x face                       +u = -z, +v = +y
         half_x,  half_y,  half_z,      u_min_z, v_max_y,    1.f,  0.f,  0.f,
         half_x, -half_y, -half_z,      u_max_z, v_min_y,    1.f,  0.f,  0.f,
         half_x,  half_y, -half_z,      u_max_z, v_max_y,    1.f,  0.f,  0.f,

         half_x, -half_y, -half_z,      u_max_z, v_min_y,    1.f,  0.f,  0.f,
         half_x,  half_y,  half_z,      u_min_z, v_max_y,    1.f,  0.f,  0.f,
         half_x, -half_y,  half_z,      u_min_z, v_min_y,    1.f,  0.f,  0.f,
        //-x face                       +u = z, +v = +y
        -half_x,  half_y,  half_z,      u_max_z, v_max_y,   -1.f,  0.f,  0.f,
        -half_x,  half_y, -half_z,      u_min_z, v_max_y,   -1.f,  0.f,  0.f,
        -half_x, -half_y, -half_z,      u_min_z, v_min_y,   -1.f,  0.f,  0.f,

        -half_x, -half_y, -half_z,      u_min_z, v_min_y,   -1.f,  0.f,  0.f,
        -half_x, -half_y,  half_z,      u_max_z, v_min_y,   -1.f,  0.f,  0.f,
        -half_x,  half_y,  half_z,      u_max_z, v_max_y,   -1.f,  0.f,  0.f,
        //+y face                       +u = +x, +v = -z
         half_x,  half_y,  half_z,      u_max_x, v_min_z,    0.f,  1.f,  0.f,
         half_x,  half_y, -half_z,      u_max_x, v_max_z,    0.f,  1.f,  0.f,
        -half_x,  half_y, -half_z,      u_min_x, v_max_z,    0.f,  1.f,  0.f,
        
        -half_x,  half_y, -half_z,      u_min_x, v_max_z,    0.f,  1.f,  0.f,
        -half_x,  half_y,  half_z,      u_min_x, v_min_z,    0.f,  1.f,  0.f,
         half_x,  half_y,  half_z,      u_max_x, v_min_z,    0.f,  1.f,  0.f,
        //-y face                       +u = +x, +v = +z
         half_x, -half_y,  half_z,      u_max_x, v_max_z,    0.f, -1.f,  0.f,
        -half_x, -half_y, -half_z,      u_min_x, v_min_z,    0.f, -1.f,  0.f,
         half_x, -half_y, -half_z,      u_max_x, v_min_z,    0.f, -1.f,  0.f,

        -half_x, -half_y, -half_z,      u_min_x, v_min_z,    0.f, -1.f,  0.f,
         half_x, -half_y,  half_z,      u_max_x, v_max_z,    0.f, -1.f,  0.f,
        -half_x, -half_y,  half_z,      u_min_x, v_max_z,    0.f, -1.f,  0.f,
        //+z face                       +u = +x, +v = +y
         half_x,  half_y,  half_z,      u_max_x, v_max_y,    0.f,  0.f,  1.f,
        -half_x, -half_y,  half_z,      u_min_x, v_min_y,    0.f,  0.f,  1.f,
         half_x, -half_y,  half_z,      u_max_x, v_min_y,    0.f,  0.f,  1.f,
        
        -half_x, -half_y,  half_z,      u_min_x, v_min_y,    0.f,  0.f,  1.f,
         half_x,  half_y,  half_z,      u_max_x, v_max_y,    0.f,  0.f,  1.f,
        -half_x,  half_y,  half_z,      u_min_x, v_max_y,    0.f,  0.f,  1.f,
        //-z face                       +u = -x, +v = +y
         half_x,  half_y, -half_z,      u_min_x, v_max_y,    0.f,  0.f, -1.f,
         half_x, -half_y, -half_z,      u_min_x, v_min_y,    0.f,  0.f, -1.f,
        -half_x, -half_y, -half_z,      u_max_x, v_min_y,    0.f,  0.f, -1.f,
        
        -half_x, -half_y, -half_z,      u_max_x, v_min_y,    0.f,  0.f, -1.f,
        -half_x,  half_y, -half_z,      u_max_x, v_max_y,    0.f,  0.f, -1.f,
         half_x,  half_y, -half_z,      u_min_x, v_max_y,    0.f,  0.f, -1.f,
    };

    return generateVBOfromData3D<sizeof(whole_data) / sizeof(whole_data[0])>(whole_data, texcoords, normals);
}

Meshes::VBO Meshes::generateQuadVBO(glm::vec2 mesh_scale, glm::vec2 texture_world_size,
                                    Meshes::TexcoordStyle style, bool normals)
{
    // generates quad vertex data with given size mesh_scale facing in the positive z axis and uploads them into vbo,
    // optionaly also generates texcoords when style is not Meshes::TexcoordStyle::none,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating,
    // optionaly also generates normals,
    // returns vbo with corresponding data uploaded
    assert(mesh_scale.x > 0.f && mesh_scale.y > 0.f); // probably useless, we could have zero sized quads

    GLfloat half_x = mesh_scale.x / 2.f,
            half_y = mesh_scale.y / 2.f;
    
    // U is horizontal  -> x direction
    // V is vertical    -> y direction
    GLfloat u_min_x = 0.f, u_max_x = 1.f,
            v_min_y = 0.f, v_max_y = 1.f;
    if (style == Meshes::TexcoordStyle::repeat)
    {
        assert(texture_world_size.x > 0.f && texture_world_size.y > 0.f); // avoid divide by zero
        
        // texture_world_size.x -> width of the texture in world coordinates
        u_max_x = mesh_scale.x / texture_world_size.x;

        // texture_world_size.y -> height of the texture in world coordinates
        v_max_y = mesh_scale.y / texture_world_size.y;
    }

    // include texcoords when style given a Texcoord style
    bool texcoords = (style != Meshes::TexcoordStyle::none);

    // counter-clockwise vertex winding order
    GLfloat whole_data[] =
    {
        // Position                  // Texcoords        // Normal
        //+z face                    +u = +x, +v = +y
         half_x,  half_y,  0.f,      u_max_x, v_max_y,    0.f,  0.f,  1.f,
        -half_x, -half_y,  0.f,      u_min_x, v_min_y,    0.f,  0.f,  1.f,
         half_x, -half_y,  0.f,      u_max_x, v_min_y,    0.f,  0.f,  1.f,

        -half_x, -half_y,  0.f,      u_min_x, v_min_y,    0.f,  0.f,  1.f,
         half_x,  half_y,  0.f,      u_max_x, v_max_y,    0.f,  0.f,  1.f,
        -half_x,  half_y,  0.f,      u_min_x, v_max_y,    0.f,  0.f,  1.f,
    };

    return generateVBOfromData3D<sizeof(whole_data) / sizeof(whole_data[0])>(whole_data, texcoords, normals);
}

static std::unique_ptr<GLfloat[]> combineBuffers(size_t vertex_count, Meshes::AttributeConfig attr_config,
                                                 GLfloat pos[], GLfloat texcoords[], GLfloat normals[])
{
    const unsigned int stride = attr_config.sum();
    if (vertex_count == 0 || stride == 0) return std::unique_ptr<GLfloat[]>();

    GLfloat *result = new GLfloat[stride * vertex_count];
    if (!result) return std::unique_ptr<GLfloat[]>();

    size_t test_idx = 1;

    GLfloat *vert = result;
    for (size_t i = 0; i < vertex_count; ++i)
    {
        if (attr_config.pos_amount > 0)
        {
            // if (i == test_idx) printf("vert-offset pos: %d, copying: %f|%f|%f\n", vert - result, pos[i + 0], pos[i + 1], pos[i + 2]);
            memcpy(vert, pos, attr_config.pos_amount * sizeof(GLfloat));
            vert += attr_config.pos_amount;
            pos += attr_config.pos_amount;
        }

        if (attr_config.texcoord_amount > 0)
        {
            // if (i == test_idx) printf("vert-offset texcoord: %d, copying: %f|%f\n", vert - result, texcoords[i + 0], texcoords[i + 1]);
            memcpy(vert, texcoords, attr_config.texcoord_amount * sizeof(GLfloat));
            vert += attr_config.texcoord_amount;
            texcoords += attr_config.texcoord_amount;
        }

        if (attr_config.normal_amount > 0)
        {
            // if (i == test_idx) printf("vert-offset normal: %d, copying: %f|%f|%f\n", vert - result, normals[i + 0], normals[i + 1], normals[i + 2]);
            memcpy(vert, normals, attr_config.normal_amount * sizeof(GLfloat));
            vert += attr_config.normal_amount;
            normals += attr_config.normal_amount;
        }
    }
    assert(vert - result == stride * vertex_count);

    return std::unique_ptr<GLfloat[]>(result);
}

int Meshes::Mesh::loadFromObj(const char *obj_file_path)
{
    //TODO assert that nothing was loaded before
    std::unique_ptr<char[]> file_content;

    tinyobj_attrib_t attrib = { 0 };
    tinyobj_shape_t *shapes = NULL;
    size_t num_shapes = 0;
    tinyobj_material_t *materials = NULL;
    size_t num_materials = 0;
    //TODO check the file loading
    file_reader_callback file_reader = [](void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len){
        if (is_mtl)
        {
            //TODO implement this + err
            puts("is_mtl case is unimplemented");
            return;
        }

        std::unique_ptr<char[]>* file_content = (std::unique_ptr<char[]>*)ctx;

        *file_content = Utils::getTextFileAsString(filename, len); //TODO check for NULL

        *buf = file_content->get();
    };
    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;

    int tinyobj_ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                                        obj_file_path, file_reader, (void*)&file_content, flags);
    if (tinyobj_ret != TINYOBJ_SUCCESS)
    {
        //TODO err
        fprintf(stderr, "Failed to parse .obj file at path: '%s'! Error: %d\n", obj_file_path, tinyobj_ret);
        return 1;
    }

    printf("Shapes: %zu\n", num_shapes);
    for (size_t i = 0; i < num_shapes; ++i) printf("Shape name: %s, offset: %d, length: %d\n", shapes[i].name, shapes[i].face_offset, shapes[i].length);
    printf("Materials: %zu\n", num_materials);
    printf("Faces: %d\n", attrib.num_faces);

    if (num_shapes == 0)
    {
        //TODO err
        return 2;
    }

    //TODO check if those are correct
    m_triangle_count = attrib.num_face_num_verts; //this is weird
    m_vert_count = m_triangle_count * 3; //probably calculate this from the iteration

    m_positions.reserve(m_vert_count * 3); // positions have 3 floats
    m_texcoords.reserve(m_vert_count * 2); // texcoords have 2 floats
    m_normals.reserve(m_vert_count * 3);   // normals have 3 floats

    // unsigned int loaded_vertex_count = 0;

    // for (unsigned int ti = 0; ti < m_triangle_count; ++ti)
    // {
    //     // Get indices for the face (triangle)
    //     tinyobj_vertex_index_t idx0 = attrib.faces[3 * ti + 0];
    //     tinyobj_vertex_index_t idx1 = attrib.faces[3 * ti + 1];
    //     tinyobj_vertex_index_t idx2 = attrib.faces[3 * ti + 2];

    //     // Fill vertices buffer (float) using vertex index of the face
    //     for (int v = 0; v < 3; v++)
    //     {
    //         m_positions[loaded_vertex_count + v] = attrib.vertices[idx0.v_idx*3 + v];
    //     }
    //     vCount[mm] +=3;

    //     for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx1.v_idx*3 + v]; } vCount[mm] +=3;

    //     for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx2.v_idx*3 + v]; } vCount[mm] +=3;
    // }

    const bool normals_included = attrib.num_normals > 0;
    size_t face_offset = 0;

    for (unsigned int fi = 0; fi < attrib.num_face_num_verts; ++fi)
    {
        const unsigned int face_num_verts = attrib.face_num_verts[fi];
        assert(face_num_verts % 3 == 0); // assume all triangle faces

        for (int t = 0; t < face_num_verts / 3; ++t)
        {
            glm::vec3 triangle_normal(0.f);

            for (int ti = 0; ti < 3; ++ti)
            {
                const tinyobj_vertex_index_t& idx = attrib.faces[face_offset + 3 * t + ti];

                //position
                int pos_idx = idx.v_idx;
                assert(pos_idx >= 0);
                assert(pos_idx < (int)attrib.num_vertices);
                m_positions.push_back(attrib.vertices[3 * pos_idx + 0]);
                m_positions.push_back(attrib.vertices[3 * pos_idx + 1]);
                m_positions.push_back(attrib.vertices[3 * pos_idx + 2]);

                //TODO texcoords
                m_texcoords.push_back(0.f);
                m_texcoords.push_back(0.f);

                //normal
                int norm_idx = idx.vn_idx;
                assert(norm_idx < (int)attrib.num_normals);

                glm::vec3 vert_normal;
                if (normals_included && norm_idx >= 0)
                {
                    vert_normal = glm::vec3(attrib.normals[3 * norm_idx + 0],
                                            attrib.normals[3 * norm_idx + 1],
                                            attrib.normals[3 * norm_idx + 2]);
                    if (ti == 0) triangle_normal = vert_normal;
                }
                else if (ti == 0)
                {
                    //TODO calculate vert_normal from face itself and set triangle_normal variable
                    assert(false);
                }
                else
                {
                    vert_normal = triangle_normal;
                }

                m_normals.push_back(vert_normal.x);
                m_normals.push_back(vert_normal.y);
                m_normals.push_back(vert_normal.z);
            }
        }

        face_offset += static_cast<size_t>(face_num_verts);
    }

    assert(m_vert_count * 3 == m_positions.size());
    assert(m_vert_count * 2 == m_texcoords.size());
    assert(m_vert_count * 3 == m_normals.size());

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    // size_t test_idx = 1;
    // printf("separated - pos: %f|%f|%f, texcoords: %f|%f, normals: %f|%f|%f\n",
    //         m_positions[test_idx * 3 + 0], m_positions[test_idx * 3 + 1], m_positions[test_idx * 3 + 2],
    //         m_texcoords[test_idx * 2 + 0], m_texcoords[test_idx * 2 + 1],
    //         m_normals[test_idx * 3 + 0], m_normals[test_idx * 3 + 1], m_normals[test_idx * 3 + 2]);

    if (!upload())
    {
        //TODO err
        fprintf(stderr, "Failed to upload loaded mesh into GPU!\n");
        return 3;
    }

    return 0;
}

bool Meshes::Mesh::upload()
{
    //setting the attribute config
    AttributeConfig attr_config{Meshes::attribute3d_pos_amount, 0, 0};

    // position attribute is always required
    assert(m_positions.size() > 0);
    assert(m_positions.size() >= m_vert_count * Meshes::attribute3d_pos_amount);

    // texcoords are optional
    if (m_texcoords.size() > 0)
    {
        assert(m_texcoords.size() >= m_vert_count * Meshes::attribute3d_texcoord_amount);
        attr_config.texcoord_amount = Meshes::attribute3d_texcoord_amount;
    }

    // normal is optional
    if (m_normals.size() > 0)
    {
        assert(m_normals.size() >= m_vert_count * Meshes::attribute3d_normal_amount);
        attr_config.normal_amount = Meshes::attribute3d_normal_amount;
    }

    //combining buffers
    std::unique_ptr<GLfloat[]> combined_data = combineBuffers(m_vert_count, attr_config,
                                                              m_positions.data(), m_texcoords.data(), m_normals.data());
    if (!combined_data)
    {
        fprintf(stderr, "Failed to combine position, texcoord, normal buffers! Most likely wrong data or out of memory.\n");
        return false;
    }

    // printf("attr_config - pos: %d, texcoords: %d, normals: %d\n", attr_config.pos_amount, attr_config.texcoord_amount, attr_config.normal_amount);
    // size_t test_idx = 1;
    // printf("combined - pos: %f|%f|%f, texcoords: %f|%f, normals: %f|%f|%f\n",
    //         combined_data[test_idx * 8 + 0], combined_data[test_idx * 8 + 1], combined_data[test_idx * 8 + 2],
    //         combined_data[test_idx * 8 + 3], combined_data[test_idx * 8 + 4],
    //         combined_data[test_idx * 8 + 5], combined_data[test_idx * 8 + 6], combined_data[test_idx * 8 + 7]);

    assert(m_vbo.m_id == empty_id);
    m_vbo.~VBO(); // just to be sure
    new (&m_vbo) VBO(combined_data.get(), m_vert_count, attr_config);

    if (m_vbo.m_id == empty_id)
    {
        fprintf(stderr, "Failed to upload geometry data into VBO.\n");
        return false;
    }

    return true;
}

bool Meshes::Mesh::isUploaded() const
{
    return m_vbo.m_id != empty_id;
}

void Meshes::Mesh::draw() const
{
    assert(isUploaded());

    m_vbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, m_vbo.vertexCount());
        // glDrawArrays(GL_TRIANGLES, 0, 3);
    m_vbo.unbind(); //TODO unbinding is an OpenGL anti-patter
}


Meshes::VBO Meshes::unit_quad_pos_only;
// Meshes::VBO Meshes::unit_quad_pos_uv_only;

bool Meshes::initBasicMeshes()
{
    assert(unit_quad_pos_only.m_id == empty_id);

    unit_quad_pos_only = std::move(generateQuadVBO(glm::vec2(1.f), glm::vec2(0.f), Meshes::TexcoordStyle::none, false));

    // unit_quad_pos_uv_only = std::move(generateQuadVBO(glm::vec2(1.f), glm::vec2(0.f), Meshes::TexcoordStyle::stretch, false));

    return unit_quad_pos_only.m_id != empty_id;
        //    unit_quad_pos_uv_only.m_id != empty_id;
}

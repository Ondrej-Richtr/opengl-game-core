#include "game.hpp"
#include "tinyobj_loader_c.h"

#include "glm/gtc/matrix_transform.hpp" // IWYU pragma: keep // translate, scale


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

static std::array<GLfloat, 6*6*Meshes::attribute3d_complete_amount> generateCubicGeometryData(glm::vec3 scale,
                                                                                              glm::vec2 texture_world_size,
                                                                                              Meshes::TexcoordStyle style)
{
    // generates cubic vertex data with given size `scale` and returns it as an array,
    // also generates texcoords, if `style` is Meshes::TexcoordStyle::none it makes them all zeroes,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating.
    assert(scale.x > 0.f && scale.y > 0.f && scale.z > 0.f); // probably useless, we could have zero sized cubes

    GLfloat half_x = scale.x / 2.f,
            half_y = scale.y / 2.f,
            half_z = scale.z / 2.f;
    
    // U is horizontal  -> only in x and z direction
    // V is vertical    -> only in y and z direction
    GLfloat u_min_x = 0.f, u_max_x = 0.f,
            u_min_z = 0.f, u_max_z = 0.f,
            v_min_y = 0.f, v_max_y = 0.f,
            v_min_z = 0.f, v_max_z = 0.f;
    if (style == Meshes::TexcoordStyle::stretch)
    {
        u_max_x = 1.f;
        u_max_z = 1.f;

        v_max_y = 1.f;
        v_max_z = 1.f;
    }
    else if (style == Meshes::TexcoordStyle::repeat)
    {
        assert(texture_world_size.x > 0.f && texture_world_size.y > 0.f); // avoid divide by zero
        
        // texture_world_size.x -> width of the texture in world coordinates
        // texture_world_size.y -> height of the texture in world coordinates
        u_max_x = scale.x / texture_world_size.x;
        u_max_z = scale.z / texture_world_size.x;

        v_max_y = scale.y / texture_world_size.y;
        v_max_z = scale.z / texture_world_size.y;
    }

    // printf("min/max u_x: %f|%f, u_z: %f|%f\n", u_min_x, u_max_x, u_min_z, u_max_z);
    // printf("min/max v_y: %f|%f, v_z: %f|%f\n", v_min_y, v_max_y, v_min_z, v_max_z);
    
    return std::array<GLfloat, 6*6*Meshes::attribute3d_complete_amount>
    // counter-clockwise vertex winding order
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
}

static std::array<GLfloat, 6*Meshes::attribute3d_complete_amount> generateQuadGeometryData(glm::vec2 scale,
                                                                                           glm::vec2 texture_world_size,
                                                                                           Meshes::TexcoordStyle style)
{
    // generates quad vertex data with given size `scale` facing in the positive z axis,
    // also generates texcoords, if `style` is Meshes::TexcoordStyle::none it makes them all zeroes,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating.
    assert(scale.x > 0.f && scale.y > 0.f); // probably useless, we could have zero sized quads

    GLfloat half_x = scale.x / 2.f,
            half_y = scale.y / 2.f;
    
    // U is horizontal  -> x direction
    // V is vertical    -> y direction
    GLfloat u_min_x = 0.f, u_max_x = 0.f,
            v_min_y = 0.f, v_max_y = 0.f;
    if (style == Meshes::TexcoordStyle::stretch)
    {
        u_max_x = 1.f;

        v_max_y = 1.f;
    }
    else if (style == Meshes::TexcoordStyle::repeat)
    {
        assert(texture_world_size.x > 0.f && texture_world_size.y > 0.f); // avoid divide by zero
        
        // texture_world_size.x -> width of the texture in world coordinates
        u_max_x = scale.x / texture_world_size.x;

        // texture_world_size.y -> height of the texture in world coordinates
        v_max_y = scale.y / texture_world_size.y;
    }
       
    return std::array<GLfloat, 6*Meshes::attribute3d_complete_amount>
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
    // returns vbo with corresponding data uploaded, on error returns vbo with empty id
    std::array<GLfloat, 6*6*Meshes::attribute3d_complete_amount> whole_data = generateCubicGeometryData(mesh_scale, texture_world_size, style);

    // include texcoords when given a Texcoord style
    bool texcoords = (style != Meshes::TexcoordStyle::none);

    return generateVBOfromData3D<whole_data.size()>(whole_data.data(), texcoords, normals);
}

Meshes::VBO Meshes::generateQuadVBO(glm::vec2 mesh_scale, glm::vec2 texture_world_size,
                                    Meshes::TexcoordStyle style, bool normals)
{
    // generates quad vertex data with given size mesh_scale facing in the positive z axis and uploads them into vbo,
    // optionaly also generates texcoords when style is not Meshes::TexcoordStyle::none,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating,
    // optionaly also generates normals,
    // returns vbo with corresponding data uploaded, on error returns vbo with empty id
    std::array<GLfloat, 6*Meshes::attribute3d_complete_amount> whole_data = generateQuadGeometryData(mesh_scale, texture_world_size, style);

    // include texcoords when given a Texcoord style
    bool texcoords = (style != Meshes::TexcoordStyle::none);

    return generateVBOfromData3D<whole_data.size()>(whole_data.data(), texcoords, normals);
}

static std::unique_ptr<GLfloat[]> combineBuffers(size_t vertex_count, Meshes::AttributeConfig attr_config,
                                                 GLfloat pos[], GLfloat texcoords[], GLfloat normals[])
{
    const unsigned int stride = attr_config.sum();
    if (vertex_count == 0 || stride == 0) return std::unique_ptr<GLfloat[]>();

    GLfloat *result = new GLfloat[stride * vertex_count];
    if (!result) return std::unique_ptr<GLfloat[]>();

    GLfloat *vert = result;
    for (size_t i = 0; i < vertex_count; ++i)
    {
        if (attr_config.pos_amount > 0)
        {
            memcpy(vert, pos, attr_config.pos_amount * sizeof(GLfloat));
            vert += attr_config.pos_amount;
            pos += attr_config.pos_amount;
        }

        if (attr_config.texcoord_amount > 0)
        {
            memcpy(vert, texcoords, attr_config.texcoord_amount * sizeof(GLfloat));
            vert += attr_config.texcoord_amount;
            texcoords += attr_config.texcoord_amount;
        }

        if (attr_config.normal_amount > 0)
        {
            memcpy(vert, normals, attr_config.normal_amount * sizeof(GLfloat));
            vert += attr_config.normal_amount;
            normals += attr_config.normal_amount;
        }
    }
    assert(vert - result == stride * vertex_count);

    return std::unique_ptr<GLfloat[]>(result);
}

Meshes::Mesh::Mesh(unsigned int vert_count, std::vector<GLfloat>&& positions,
                   std::vector<GLfloat>&& texcoords, std::vector<GLfloat>&& normals)
                : m_vert_count(0), m_triangle_count(0), m_positions(), m_texcoords(), m_normals(), m_vbo()
{
    // ignoring the return value as the caller should check anyways with call to `isUploaded`
    loadFromData(vert_count, std::move(positions), std::move(texcoords), std::move(normals));
}

int Meshes::Mesh::loadFromData(unsigned int vert_count, std::vector<GLfloat>&& positions,
                               std::vector<GLfloat>&& texcoords, std::vector<GLfloat>&& normals)
{
    assert(m_vert_count == 0);
    assert(m_triangle_count == 0);
    assert(m_positions.size() == 0);
    assert(m_texcoords.size() == 0);
    assert(m_normals.size() == 0);

    if (!vert_count)
    {
        fprintf(stderr, "Failed to load mesh from data as the vertex count is 0!\n");
        return 1;
    }
    
    if (vert_count % 3 != 0)
    {
        fprintf(stderr, "Failed to load mesh from data as the vertex count(%d) is not divisble by 3!\n", vert_count);
        return 2;
    }

    if (vert_count * 3 < positions.size() ||
        vert_count * 2 < texcoords.size() ||
        vert_count * 3 < normals.size())
    {
        fprintf(stderr, "Failed to load mesh from data as the vectors does not contain enough data!\n");
        return 3;
    }

    assert(vert_count * 3 == positions.size());
    assert(vert_count * 2 == texcoords.size());
    assert(vert_count * 3 == normals.size());

    m_positions = std::move(positions);
    m_texcoords = std::move(texcoords);
    m_normals = std::move(normals);

    m_vert_count = vert_count;
    m_triangle_count = m_vert_count / 3;

    if (!upload())
    {
        fprintf(stderr, "Failed to upload loaded mesh from data into GPU!\n");
        // intentionally not clearing vectors and other stuff as we might try to upload later
        return 4;
    }

    return 0;
}

int Meshes::Mesh::loadFromObj(const char *obj_file_path)
{
    assert(obj_file_path);
    assert(m_vert_count == 0);
    assert(m_triangle_count == 0);
    assert(m_positions.size() == 0);
    assert(m_texcoords.size() == 0);
    assert(m_normals.size() == 0);
    
    if (Meshes::loadObj(obj_file_path, &m_vert_count, &m_triangle_count,
                        m_positions, m_texcoords, m_normals, NULL)) // NULL for not loading materials
    {
        return 1; // error is printed inside of loadObj
    }
    assert(m_vert_count > 0);
    assert(m_triangle_count > 0);
    assert(m_positions.size() > 0);
    assert(m_texcoords.size() > 0);
    assert(m_normals.size() > 0);

    if (!upload())
    {
        fprintf(stderr, "Failed to upload loaded mesh from .obj file '%s' into GPU!\n", obj_file_path);
        // intentionally not clearing vectors and other stuff as we might try to upload later
        return 2;
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
    m_vbo.unbind(); //TODO unbinding is an OpenGL anti-patter
}

template <size_t whole_data_len>
static Meshes::Mesh generateMeshfromData3D(const GLfloat *whole_data)
{
    // constructs Mesh with vertex data given in whole_data,
    // whole_data MUST have the full usual layout in correct order!
    // also the whole_data array must be of length whole_data_len!

    // layout of whole_data must be (in this order): 3 floats for position + 2 floats for texcoords + 3 floats for normal
    static_assert(whole_data_len % 8 == 0,
                  "`generateMeshfromData3D` function requires whole_data with length divisible by 8! (8 == 3 for vertices + 2 for uv + 3 for normals)");
    static_assert(whole_data_len > 0, "`generateMeshfromData3D` function requires non-empty whole_data!"); // empty data makes no sense
    constexpr unsigned int vert_count = static_cast<unsigned int>(whole_data_len / 8);

    std::vector<GLfloat> positions{}, texcoords{}, normals{};

    for (unsigned int i = 0; i < vert_count; ++i)
    {
        positions.push_back(whole_data[0]);
        positions.push_back(whole_data[1]);
        positions.push_back(whole_data[2]);

        texcoords.push_back(whole_data[3]);
        texcoords.push_back(whole_data[4]);

        normals.push_back(whole_data[5]);
        normals.push_back(whole_data[6]);
        normals.push_back(whole_data[7]);

        whole_data += 8;
    }

    assert(vert_count * 3 == positions.size());
    assert(vert_count * 2 == texcoords.size());
    assert(vert_count * 3 == normals.size());

    return Meshes::Mesh(vert_count, std::move(positions), std::move(texcoords), std::move(normals));
}

Meshes::Mesh Meshes::generateCubicMesh(glm::vec3 mesh_scale,  glm::vec2 texture_world_size, Meshes::TexcoordStyle style)
{
    // generates cubic vertex data with given size `mesh_scale` and uploads them into mesh,
    // also generates texcoords, if style is Meshes::TexcoordStyle::none it makes them all zeroes,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating,
    // also generates normals,
    // returns mesh with corresponding data uploaded, on error returns mesh with isUploaded == false
    std::array<GLfloat, 6*6*Meshes::attribute3d_complete_amount> whole_data = generateCubicGeometryData(mesh_scale, texture_world_size, style);

    return generateMeshfromData3D<whole_data.size()>(whole_data.data());
}
    
Meshes::Mesh Meshes::generateQuadMesh(glm::vec2 mesh_scale, glm::vec2 texture_world_size, Meshes::TexcoordStyle style)
{
    // generates quad vertex data with given size `mesh_scale` and uploads them into mesh,
    // also generates texcoords, if style is Meshes::TexcoordStyle::none it makes them all zeroes,
    // if Meshes::TexcoordStyle::repeat then texture_world_size is used to deduce UV repeating,
    // also generates normals,
    // returns mesh with corresponding data uploaded, on error returns mesh with isUploaded == false
    std::array<GLfloat, 6*Meshes::attribute3d_complete_amount> whole_data = generateQuadGeometryData(mesh_scale, texture_world_size, style);

    return generateMeshfromData3D<whole_data.size()>(whole_data.data());
}

Meshes::Model::Model(const Shaders::Program& shader, const Meshes::Mesh& mesh, Lighting::Material material)
                : m_shader(shader), m_material(material), m_mesh(mesh),
                  m_origin_offset(0.f), m_translate(0.f), m_scale(1.f) {}

void Meshes::Model::draw(const Drawing::Camera3D& camera, const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                         float gamma, glm::vec3 pos, glm::vec3 scale) const
{
    m_shader.use();

    //vs
    glm::mat4 model_mat(1.f);
    model_mat = glm::translate(model_mat, m_translate + pos);
    model_mat = glm::scale(model_mat, m_scale * scale);
    model_mat = glm::translate(model_mat, m_origin_offset);

    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

    m_shader.set("model", model_mat);
    m_shader.set("normalMat", normal_mat);
    m_shader.set("view", camera.getViewMatrix());
    m_shader.set("projection", camera.getProjectionMatrix());

    //fs
    m_shader.set("cameraPos", camera.m_pos);
    m_shader.setMaterial(m_material);
    
    int lights_set = m_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights);
    assert(lights_set >= 0);
    assert((size_t)lights_set <= lights.size());
    if ((size_t)lights_set < lights.size())
    {
        fprintf(stderr, "[WARNING] Not all lights were attached to the shader! Wanted amount: %zu, set amount: %d\n.",
                lights.size(), lights_set);
    }
    
    m_shader.set("gammaCoef", gamma);

    m_mesh.draw();
}

void Meshes::Model::drawWithColorTint(const Drawing::Camera3D& camera,
                                      const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                                      float gamma, glm::vec3 pos, const Color3F color_tint, glm::vec3 scale) const
{
    m_shader.use();

    //vs
    glm::mat4 model_mat(1.f);
    model_mat = glm::translate(model_mat, m_translate + pos);
    model_mat = glm::scale(model_mat, m_scale * scale);
    model_mat = glm::translate(model_mat, m_origin_offset);

    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

    m_shader.set("model", model_mat);
    m_shader.set("normalMat", normal_mat);
    m_shader.set("view", camera.getViewMatrix());
    m_shader.set("projection", camera.getProjectionMatrix());

    //fs
    Lighting::Material tinted_material = m_material;
    // tinted_material.m_props.m_ambient = Drawing::blendScreen(m_material.m_props.m_ambient, color_tint);
    // tinted_material.m_props.m_diffuse = Drawing::blendScreen(m_material.m_props.m_diffuse, color_tint);
    tinted_material.m_props.m_ambient = m_material.m_props.m_ambient.mult(color_tint);
    tinted_material.m_props.m_diffuse = m_material.m_props.m_diffuse.mult(color_tint);

    m_shader.set("cameraPos", camera.m_pos);
    m_shader.setMaterial(tinted_material);
    
    int lights_set = m_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights);
    assert(lights_set >= 0);
    assert((size_t)lights_set <= lights.size());
    if ((size_t)lights_set < lights.size())
    {
        fprintf(stderr, "[WARNING] Not all lights were attached to the shader! Wanted amount: %zu, set amount: %d\n.",
                lights.size(), lights_set);
    }

    m_shader.set("gammaCoef", gamma);

    m_mesh.draw();
}

//Loads geometry and material data out of .obj files with usage of `tinyobj_loader_c`, returns 0 when success, non-zero when error.
//Optionally can load materials as well.
int Meshes::loadObj(const char *obj_file_path, unsigned int *out_vert_count, unsigned int *out_triangle_count,
                    std::vector<GLfloat>& out_positions, std::vector<GLfloat>& out_texcoords, std::vector<GLfloat>& out_normals,
                    std::vector<Lighting::MaterialProps> *out_material_props)
{
    std::array<std::unique_ptr<char[]>, 2> file_reader_ctx{}; // index 0 is for .obj file contents and index 1 for .mtl file contents
    file_reader_callback file_reader = [](void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len)
    {
        std::array<std::unique_ptr<char[]>, 2>& file_reader_ctx = *(std::array<std::unique_ptr<char[]>, 2>*)ctx;
        
        int idx = static_cast<int>(is_mtl != 0); // 0 when not is_mtl, 1 when is_mtl

        if (file_reader_ctx[idx])
        {
            fprintf(stderr, "[WARNING] Failed to load file: '%s' during loading of mesh '%s' as this type of file(%d) was already loaded!\n",
                    filename, obj_filename, idx);
            return;
        }

        file_reader_ctx[idx] = Utils::getTextFileAsString(filename, len); //TODO check for NULL
        *buf = file_reader_ctx[idx].get();
    };

    tinyobj_attrib_t attrib = { 0 };
    tinyobj_shape_t *shapes = NULL;
    size_t num_shapes = 0;
    tinyobj_material_t *materials = NULL;
    size_t num_materials = 0;
    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;

    int tinyobj_ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                                        obj_file_path, file_reader, (void*)&file_reader_ctx, flags);
    if (tinyobj_ret != TINYOBJ_SUCCESS)
    {
        fprintf(stderr, "Failed to parse .obj file at path: '%s'! Error: %d\n", obj_file_path, tinyobj_ret);
        return 1;
    }

    //DEBUG
    /*printf("Shapes: %zu\n", num_shapes);
    for (size_t i = 0; i < num_shapes; ++i) printf("Shape name: %s, offset: %d, length: %d\n", shapes[i].name, shapes[i].face_offset, shapes[i].length);
    printf("Materials: %zu\n", num_materials);
    for (size_t i = 0; i < num_materials; ++i) printf("Material name: %s, illum: %d, textures - ambient: %s, diffuse: %s, specular: %s, specular_highlight: %s\n",
                                                      materials[i].name, materials[i].illum, materials[i].ambient_texname, materials[i].diffuse_texname,
                                                      materials[i].specular_texname, materials[i].specular_highlight_texname);
    printf("Faces: %d\n", attrib.num_faces);*/

    if (num_shapes == 0)
    {
        fprintf(stderr, "Attempted to load empty .obj file at path: '%s'!", obj_file_path);
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);

        return 2;
    }

    //Geometry data
    //TODO check if those are correct
    unsigned int triangle_count = attrib.num_face_num_verts; //this is weird
    unsigned int vert_count = triangle_count * 3; //probably calculate this from the iteration

    out_positions.reserve(vert_count * 3); // positions have 3 floats
    out_texcoords.reserve(vert_count * 2); // texcoords have 2 floats
    out_normals.reserve(vert_count * 3);   // normals have 3 floats

    const bool normals_included = attrib.num_normals > 0, texcoords_included = attrib.num_texcoords > 0;
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
                out_positions.push_back(attrib.vertices[3 * pos_idx + 0]);
                out_positions.push_back(attrib.vertices[3 * pos_idx + 1]);
                out_positions.push_back(attrib.vertices[3 * pos_idx + 2]);

                //texcoords
                int tex_idx = idx.vt_idx;
                assert(tex_idx < (int)attrib.num_texcoords);

                if (texcoords_included && tex_idx >= 0)
                {
                    out_texcoords.push_back(attrib.texcoords[2 * tex_idx + 0]);
                    out_texcoords.push_back(attrib.texcoords[2 * tex_idx + 1]);
                }
                else // insert 0x0 coordinates as we want texcoords always included anyways
                {
                    out_texcoords.push_back(0.f);
                    out_texcoords.push_back(0.f);
                }

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
                    //TODO go through all normals afterwards and change each zero normal into valid normal from face position
                    assert(false);
                    vert_normal = glm::vec3(0.f);
                    triangle_normal = vert_normal;
                }
                else
                {
                    vert_normal = triangle_normal;
                }

                out_normals.push_back(vert_normal.x);
                out_normals.push_back(vert_normal.y);
                out_normals.push_back(vert_normal.z);
            }
        }

        face_offset += static_cast<size_t>(face_num_verts);
    }

    assert(vert_count * 3 == out_positions.size());
    assert(vert_count * 2 == out_texcoords.size());
    assert(vert_count * 3 == out_normals.size());

    if (out_vert_count) *out_vert_count = vert_count;
    if (out_triangle_count) *out_triangle_count = triangle_count;

    //Material data
    if (out_material_props && materials)
    {
        for (size_t i = 0; i < num_materials; ++i)
        {
            const tinyobj_material_t& mat = materials[i];
            Color3F ambient{static_cast<const GLfloat*>(mat.ambient)},
                    diffuse{static_cast<const GLfloat*>(mat.diffuse)},
                    specular{static_cast<const GLfloat*>(mat.specular)};
            out_material_props->emplace_back(ambient, diffuse, specular, mat.shininess);
        }
        assert(out_material_props->size() == num_materials);
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return 0;
}

//Loads materials from given .obj files using `tinyobj_loader_c`, returns 0 when success, non-zero when error.
int Meshes::loadMtl(const char *mtl_file_path, std::vector<Lighting::MaterialProps>& out_material_props)
{
    //TODO file contents for .obj file not needed
    std::array<std::unique_ptr<char[]>, 2> file_reader_ctx{}; // index 0 is for .obj file contents and index 1 for .mtl file contents
    file_reader_callback file_reader = [](void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len)
    {
        std::array<std::unique_ptr<char[]>, 2>& file_reader_ctx = *(std::array<std::unique_ptr<char[]>, 2>*)ctx;
        
        int idx = static_cast<int>(is_mtl != 0); // 0 when not is_mtl, 1 when is_mtl

        if (file_reader_ctx[idx])
        {
            fprintf(stderr, "[WARNING] Failed to load file: '%s' during loading of material data as this type of file(%d) was already loaded!\n",
                    filename, idx);
            return;
        }

        file_reader_ctx[idx] = Utils::getTextFileAsString(filename, len); //TODO check for NULL
        *buf = file_reader_ctx[idx].get();
    };

    tinyobj_material_t *materials = NULL;
    size_t num_materials = 0;

    // NULL for no .obj file path as we only care about the .mtl file
    int tinyobj_ret = tinyobj_parse_mtl_file(&materials, &num_materials, mtl_file_path, NULL, file_reader, (void*)&file_reader_ctx);
    if (tinyobj_ret != TINYOBJ_SUCCESS)
    {
        fprintf(stderr, "Failed to parse .mtl file at path: '%s'! Error: %d\n", mtl_file_path, tinyobj_ret);
        return 1;
    }

    if (!materials || num_materials == 0)
    {
        fprintf(stderr, "No materials found at path: '%s', but parsing did not return an error!\n", mtl_file_path);
        tinyobj_materials_free(materials, num_materials);
        return 2;
    }

    //DEBUG
    // printf("Materials for '%s', material count: %zu\n", mtl_file_path, num_materials);
    // for (size_t i = 0; i < num_materials; ++i) printf("Material name: %s, illum: %d, textures - ambient: %s, diffuse: %s, specular: %s, specular_highlight: %s, bump: %s\n",
    //                                                   materials[i].name, materials[i].illum, materials[i].ambient_texname, materials[i].diffuse_texname,
    //                                                   materials[i].specular_texname, materials[i].specular_highlight_texname, materials[i].bump_texname);

    for (size_t i = 0; i < num_materials; ++i)
    {
        const tinyobj_material_t& mat = materials[i];
        //DEBUG
        /*printf("loaded material - ambient: %f|%f|%f, diffuse: %f|%f|%f, specular: %f|%f|%f, shinines: %f\n",
           mat.ambient[0], mat.ambient[1], mat.ambient[2],
           mat.diffuse[0], mat.diffuse[1], mat.diffuse[2],
           mat.specular[0], mat.specular[1], mat.specular[2], mat.shininess);*/
        
        Color3F ambient{static_cast<const GLfloat*>(mat.ambient)},
                diffuse{static_cast<const GLfloat*>(mat.diffuse)},
                specular{static_cast<const GLfloat*>(mat.specular)};
        //TODO https://www.fileformat.info/format/material/
        out_material_props.emplace_back(ambient, diffuse, specular, mat.shininess);
    }
    assert(out_material_props.size() == num_materials);

    tinyobj_materials_free(materials, num_materials);

    return 0;
}

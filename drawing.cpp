#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/ext/matrix_transform.hpp" //glm::lookAt
#include "glm/gtc/matrix_transform.hpp" //glm::perspective
#include "glm/gtx/matrix_transform_2d.hpp" //glm::translate and glm::scale


Drawing::Camera3D::Camera3D(float fov, float aspect_ration, glm::vec3 pos, glm::vec3 target,
                            float near_plane, float far_plane)
                    : m_pos(pos), m_target(target), m_view_mat(), m_proj_mat()
{
    updateViewMatrix(); // properly sets m_view_mat
    m_proj_mat = glm::perspective(glm::radians(fov), aspect_ration, near_plane, far_plane);
}

Drawing::Camera3D::Camera3D(float fov, float aspect_ration, glm::vec3 pos, float pitch, float yaw,
                            float near_plane, float far_plane)
                    : m_pos(pos), m_target(), m_view_mat(), m_proj_mat()
{
    setTargetFromPitchYaw(pitch, yaw);  // properly sets m_target and m_view_mat
    //updateViewMatrix();               // properly sets m_view_mat
    m_proj_mat = glm::perspective(glm::radians(fov), aspect_ration, near_plane, far_plane);
}

void Drawing::Camera3D::setPosition(glm::vec3 pos)
{
    if (pos != m_pos)
    {
        m_pos = pos;
        updateViewMatrix();
    }
}

void Drawing::Camera3D::movePosition(glm::vec3 move_vec)
{
    setPosition(m_pos + move_vec);
}

void Drawing::Camera3D::setTarget(glm::vec3 target)
{
    if (target != m_target)
    {
        m_target = target;
        updateViewMatrix();
    }
}

void Drawing::Camera3D::moveTarget(glm::vec3 move_vec)
{
    setTarget(m_target + move_vec);
}

void Drawing::Camera3D::setTargetFromPitchYaw(float pitch, float yaw)
{
    const float sin_pitch = sin(glm::radians(pitch));
    const float cos_pitch = cos(glm::radians(pitch));
    const float sin_yaw = sin(glm::radians(yaw));
    const float cos_yaw = cos(glm::radians(yaw));
    glm::vec3 dir = glm::normalize(glm::vec3(cos_yaw * cos_pitch, sin_pitch, sin_yaw * cos_pitch)); //TODO up_vec

    setTarget(m_pos + dir);
}

void Drawing::Camera3D::move(glm::vec3 move_vec)
{
    movePosition(move_vec);
    moveTarget(move_vec);
}

// glm::vec3 Drawing::Camera3D::coordsWorldToView(glm::vec3 vec) const
// {
//     return getViewMatrix() * vec;
// }

void Drawing::Camera3D::updateViewMatrix()
{
    m_view_mat = glm::lookAt(m_pos, m_target, Drawing::up_dir);
}

const glm::mat4& Drawing::Camera3D::getViewMatrix() const
{
    return m_view_mat;
}

const glm::mat4& Drawing::Camera3D::getProjectionMatrix() const
{
    return m_proj_mat;
}

glm::vec3 Drawing::Camera3D::dirCoordsViewToWorld(glm::vec3 dir) const
{
    glm::mat3 m(getViewMatrix()); //IDEA cache this too
    glm::vec3 dir_transformed = glm::inverse(m) * dir;

    return NORMALIZE_OR_0(dir_transformed);
}

glm::vec3 Drawing::Camera3D::getDirection() const
{
    return NORMALIZE_OR_0(m_target - m_pos);
}

Collision::Ray Drawing::Camera3D::getRay() const
{
    return Collision::Ray(m_pos, getDirection());
}

Drawing::FrameBuffer::FrameBuffer()
                        : m_id(empty_id)
{
    glGenFramebuffers(1, &m_id);
}

Drawing::FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void Drawing::FrameBuffer::bind() const
{
    assert(m_id != empty_id);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Drawing::FrameBuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, empty_id);
}

void Drawing::FrameBuffer::attachColorBuffer(Drawing::FrameBuffer::Attachment attachment) const
{
    bind();

    switch (attachment.type)
    {
    case Drawing::FrameBuffer::AttachmentType::none:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, empty_id, 0);
        break;
    case Drawing::FrameBuffer::AttachmentType::texture:
        assert(attachment.id != empty_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attachment.id, 0);
        break;
    case Drawing::FrameBuffer::AttachmentType::render:
        assert(attachment.id != empty_id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, attachment.id);
        break;
    default:
        fprintf(stderr, "Encountered unhandled case of AttachmentType in FrameBuffer::attachColorBuffer!\n");
        break;
    }
}

void Drawing::FrameBuffer::attachDepthBuffer(Drawing::FrameBuffer::Attachment attachment) const
{
    bind();

    switch (attachment.type)
    {
    case Drawing::FrameBuffer::AttachmentType::none:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, empty_id, 0);
        break;
    case Drawing::FrameBuffer::AttachmentType::texture:
        assert(attachment.id != empty_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachment.id, 0);
        break;
    case Drawing::FrameBuffer::AttachmentType::render:
        assert(attachment.id != empty_id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, attachment.id);
        break;
    default:
        fprintf(stderr, "Encountered unhandled case of AttachmentType in FrameBuffer::attachDepthBuffer!\n");
        break;
    }
}

void Drawing::FrameBuffer::attachStencilBuffer(Drawing::FrameBuffer::Attachment attachment) const
{
    bind();

    switch (attachment.type)
    {
    case Drawing::FrameBuffer::AttachmentType::none:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, empty_id, 0);
        break;
    case Drawing::FrameBuffer::AttachmentType::texture:
        assert(attachment.id != empty_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment.id, 0);
        break;
    case Drawing::FrameBuffer::AttachmentType::render:
        assert(attachment.id != empty_id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attachment.id);
        break;
    default:
        fprintf(stderr, "Encountered unhandled case of AttachmentType in FrameBuffer::attachStencilBuffer!\n");
        break;
    }
}

void Drawing::FrameBuffer::attachAll(Drawing::FrameBuffer::Attachment color,
                                     Drawing::FrameBuffer::Attachment depth,
                                     Drawing::FrameBuffer::Attachment stencil) const
{
    attachColorBuffer(color);
    attachDepthBuffer(depth);
    attachStencilBuffer(stencil);
}

bool Drawing::FrameBuffer::isComplete() const
{
    if (m_id == empty_id) return false;

    bind();

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // printf("framebuffer status: 0x%x\n", status);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

void Drawing::clear(Color color)
{
    const ColorF colorf = color.toFloat();

    glClearColor(colorf.r, colorf.g, colorf.b, colorf.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Drawing::texturedRectangle(const Shaders::Program& tex_rect_shader, const Textures::Texture2D& textureRect,
                                glm::vec2 screen_res, glm::vec2 dstPos, glm::vec2 dstSize)
{
    const Meshes::VBO& vbo = Meshes::unit_quad_pos_only;
    assert(vbo.m_id != empty_id);

    glm::mat4 transform(1.f);
    transform = glm::scale(transform, glm::vec3(2.f));

    tex_rect_shader.use();
    textureRect.bind();
    {
        //vs
        tex_rect_shader.set("transform", transform);

        //fs
        tex_rect_shader.set("screenRes", screen_res);
        tex_rect_shader.set("rectPosIn", dstPos);
        tex_rect_shader.set("rectSize", dstSize);
    }

    vbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, vbo.vertexCount());
    vbo.unbind();
}

void Drawing::texturedRectangle2(const Shaders::Program& tex_rect_shader, const Textures::Texture2D& textureRect,
                                 const Textures::Texture2D& background, const Textures::Texture2D& foreground,
                                 glm::vec2 dstPos, glm::vec2 dstSize)
{
    const Meshes::VBO& vbo = Meshes::unit_quad_pos_only;
    assert(vbo.m_id != empty_id);

    glm::mat4 transform(1.f);
    transform = glm::scale(transform, glm::vec3(2.f));

    tex_rect_shader.use();
    textureRect.bind(0);
    background.bind(1);
    foreground.bind(2);
    {
        //vs
        tex_rect_shader.set("transform", transform);

        //fs
        tex_rect_shader.set("inputTexture", 0);
        tex_rect_shader.set("inputTextureBG", 1);
        tex_rect_shader.set("inputTextureFG", 2);
        
        tex_rect_shader.set("rectPos", dstPos);
        tex_rect_shader.set("rectSize", dstSize);
    }

    vbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, vbo.vertexCount());
    vbo.unbind();
}

void Drawing::screenLine(const Shaders::Program& line_shader, const Meshes::VBO& line_vbo, glm::vec2 screen_res,
                         glm::vec2 v1, glm::vec2 v2, float thickness, ColorF color)
{
    assert(line_vbo.m_id != empty_id);
    assert(thickness >= 1.f);

    // v * scale + translation = u
    // -> translation = v1
    // -> scale = v2 - translation = v2 - v1
    const glm::vec2 t = v1;
    const glm::vec2 s = v2 - v1;

    glm::mat3 transform(1.f);
    transform = glm::translate(transform, t);
    transform = glm::scale(transform, s);

    glLineWidth(thickness);

    line_shader.use();
    {
        //vs
        line_shader.set("transform", transform);
        line_shader.set("screenRes", screen_res);

        //fs
        line_shader.set("color", color);
    }

    line_vbo.bind();
            glDrawArrays(GL_LINES, 0, line_vbo.vertexCount());
    line_vbo.unbind();
}

void Drawing::crosshair(const Shaders::Program& line_shader, const Meshes::VBO& line_vbo, glm::vec2 screen_res,
                        glm::vec2 size, glm::vec2 screen_pos, float thickness, ColorF color)
{
    // horizontal (x) line
    glm::vec2 v1_x(screen_pos.x, screen_pos.y - (size.y / 2.f));
    glm::vec2 v2_x(screen_pos.x, screen_pos.y + (size.y / 2.f));
    Drawing::screenLine(line_shader, line_vbo, screen_res, v1_x, v2_x, thickness, color);

    // vertical (y) line
    glm::vec2 v1_y(screen_pos.x - (size.x / 2.f), screen_pos.y);
    glm::vec2 v2_y(screen_pos.x + (size.x / 2.f), screen_pos.y);
    Drawing::screenLine(line_shader, line_vbo, screen_res, v1_y, v2_y, thickness, color);
}

void Drawing::target(const Shaders::Program& shader, const Drawing::Camera3D& camera,
                     const std::vector<std::reference_wrapper<const Lighting::Light>>& lights, const Game::Target& target,
                     double current_frame_time, glm::vec3 pos_offset)
{
    glm::vec2 target_size = target.getSize(current_frame_time);

    shader.use();
    target.m_texture.bind();
    {
        //vs
        glm::mat4 model_mat(1.f);
        model_mat = glm::translate(model_mat, target.m_pos + pos_offset);
        // assumess that target's vbo is a quad facing in z axis
        model_mat = glm::scale(model_mat, glm::vec3(target_size.x, target_size.y, 1.f));

        glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

        shader.set("model", model_mat);
        shader.set("normalMat", normal_mat);
        shader.set("view", camera.getViewMatrix());
        shader.set("projection", camera.getProjectionMatrix());

        //fs
        shader.set("cameraPos", camera.m_pos);
        shader.setMaterialProps(target.m_material);
        
        int lights_set = shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights);
        assert(lights_set >= 0);
        assert((size_t)lights_set <= lights.size());
        if ((size_t)lights_set < lights.size())
        {
            fprintf(stderr, "[WARNING] Not all lights were attached to the shader! Wanted amount: %zu, set amount: %d\n.",
                    lights.size(), lights_set);
        }
    }

    target.m_vbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, target.m_vbo.vertexCount());
    target.m_vbo.unbind();
}

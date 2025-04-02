#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


std::optional<SharedGLContext> SharedGLContext::instance{};

SharedGLContext::SharedGLContext(bool use_fbo3d, unsigned int init_width, unsigned int init_height)
                    : fbo3d_tex(init_width, init_height, GL_RGB),
                      fbo3d_rbo_depth(empty_id), //fbo3d_rbo_stencil(empty_id),
                      fbo3d(), use_fbo3d(use_fbo3d)
{
    assert(!Utils::checkForGLError());

    if (fbo3d_tex.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create FrameBuffer color texture for 3D rendering!\n");
        return;
    }

    //TODO render buffer object abstraction
    {
        assert(!Utils::checkForGLError());

        GLuint rbos[1];
        glGenRenderbuffers(1, rbos);
        if (Utils::checkForGLError())
        {
            fprintf(stderr, "Failed to create RenderBuffers for 3D FrameBuffer!\n");
            return;
        }

        fbo3d_rbo_depth = rbos[0];
        // fbo3d_rbo_stencil = rbos[1];
    }

    //depth renderbuffer allocation
    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo3d_tex.m_width, fbo3d_tex.m_height);

    //stencil renderbuffer allocation
    //TODO allow OpenGL 3.3 on dekstops to make stencil buffers work even on nvidia cards
    // glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo3d_tex.m_width, fbo3d_tex.m_height);

    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);


    //initializing framebuffer itself
    using FrameBuffer = Drawing::FrameBuffer;

    fbo3d.init();
    if (fbo3d.m_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize FrameBuffer for 3D scene!\n");
        return;
    }

    fbo3d.attachAll(fbo3d_tex.asFrameBufferAttachment(),
                    FrameBuffer::Attachment{ fbo3d_rbo_depth, FrameBuffer::AttachmentType::render },
                    // FrameBuffer::Attachment{ fbo3d_rbo_stencil, FrameBuffer::AttachmentType::render }
                    FrameBuffer::Attachment{ 0, FrameBuffer::AttachmentType::none } //TODO fix stencil buffer not working on nvidia
                    );
    if (!fbo3d.isComplete())
    {
        fprintf(stderr, "FrameBuffer for 3D scene is not complete!\n");
        fbo3d.deinit();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, empty_id);
}

SharedGLContext::~SharedGLContext()
{
    glDeleteBuffers(1, &fbo3d_rbo_depth);
    // glDeleteBuffers(1, &fbo3d_rbo_stencil);
}
bool SharedGLContext::isInitialized() const
{
    return fbo3d.m_id != empty_id;
}

glm::ivec2 SharedGLContext::getFbo3DSize() const
{
    return glm::ivec2(fbo3d_tex.m_width, fbo3d_tex.m_height);
}

void SharedGLContext::changeFbo3DSize(unsigned int new_width, unsigned int new_height)
{
    printf("Changing FBO 3D size to: %dx%d\n", new_width, new_height);

    //TODO implement check for out of memory and other OpenGL errors
    assert(!Utils::checkForGLError());

    // resize the fbo texture
    fbo3d_tex.changeTexture(new_width, new_height, GL_RGB);
    assert(!Utils::checkForGLErrorsAndPrintThem());

    // resize the fbo renderbuffers
    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, new_width, new_height);
    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);
    assert(!Utils::checkForGLErrorsAndPrintThem());

    assert(fbo3d.isComplete());
}

const Textures::Texture2D& SharedGLContext::getFbo3DTexture() const
{
    return fbo3d_tex;
}

const Drawing::FrameBuffer& SharedGLContext::getFbo3D() const
{
    return fbo3d;
}

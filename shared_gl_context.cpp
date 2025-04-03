#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


std::optional<SharedGLContext> SharedGLContext::instance{};

SharedGLContext::SharedGLContext(bool use_fbo3d, unsigned int init_width, unsigned int init_height)
                    : fbo3d_tex(init_width, init_height, GL_RGB),
                      #ifdef USE_COMBINED_FBO_BUFFERS
                        fbo3d_rbo_depth_stencil(empty_id),
                      #else
                        fbo3d_rbo_depth(empty_id),
                        fbo3d_rbo_stencil(empty_id),
                      #endif
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
        
        #ifdef USE_COMBINED_FBO_BUFFERS
            GLuint rbos[1]{};
        #else
            GLuint rbos[2]{};
        #endif
        const size_t rbos_count = sizeof(rbos)/sizeof(rbos[0]);

        glGenRenderbuffers(rbos_count, rbos);
        if (Utils::checkForGLError())
        {
            fprintf(stderr, "Failed to create RenderBuffers(%zu) for 3D FrameBuffer!\n", rbos_count);
            return;
        }

        #ifdef USE_COMBINED_FBO_BUFFERS
            fbo3d_rbo_depth_stencil = rbos[0];
        #else
            fbo3d_rbo_depth = rbos[0];
            fbo3d_rbo_stencil = rbos[1];
        #endif
    }

    #ifdef USE_COMBINED_FBO_BUFFERS
        //depth-stencil combined renderbuffer allocation
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth_stencil);
        //TODO GL_DEPTH_STENCIL or GL_DEPTH24_STENCIL8?
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, fbo3d_tex.m_width, fbo3d_tex.m_height);
    #else
        //depth renderbuffer allocation
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo3d_tex.m_width, fbo3d_tex.m_height);

        //stencil renderbuffer allocation
        //TODO allow OpenGL 3.3 on dekstops to make stencil buffers work even on nvidia cards
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo3d_tex.m_width, fbo3d_tex.m_height);
    #endif

    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);

    //initializing framebuffer itself
    using FrameBuffer = Drawing::FrameBuffer;

    fbo3d.init();
    if (fbo3d.m_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize FrameBuffer for 3D scene!\n");
        return;
    }

    #ifdef USE_COMBINED_FBO_BUFFERS
        fbo3d.attachAllCombined(fbo3d_tex.asFrameBufferAttachment(),
                                FrameBuffer::Attachment{ fbo3d_rbo_depth_stencil, FrameBuffer::AttachmentType::render });
    #else
        fbo3d.attachAllSeparated(fbo3d_tex.asFrameBufferAttachment(),
                                 FrameBuffer::Attachment{ fbo3d_rbo_depth, FrameBuffer::AttachmentType::render },
                                 // FrameBuffer::Attachment{ fbo3d_rbo_stencil, FrameBuffer::AttachmentType::render }
                                 FrameBuffer::Attachment{ 0, FrameBuffer::AttachmentType::none } //TODO fix stencil buffer not working on nvidia
                                 );
    #endif
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
    #ifdef USE_COMBINED_FBO_BUFFERS
        glDeleteBuffers(1, &fbo3d_rbo_depth_stencil);
    #else
        glDeleteBuffers(1, &fbo3d_rbo_depth);
        glDeleteBuffers(1, &fbo3d_rbo_stencil);
    #endif
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
    #ifdef USE_COMBINED_FBO_BUFFERS
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth_stencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, new_width, new_height);
        assert(!Utils::checkForGLErrorsAndPrintThem());
    #else
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, new_width, new_height);
        assert(!Utils::checkForGLErrorsAndPrintThem());

        // glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, new_width, new_height);
        // assert(!Utils::checkForGLErrorsAndPrintThem());
    #endif

    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);

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

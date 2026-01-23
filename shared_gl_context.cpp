#include "game.hpp"


std::optional<SharedGLContext> SharedGLContext::instance{};

SharedGLContext::SharedGLContext(unsigned int init_width, unsigned int init_height, unsigned int fbo3d_samples, const RenderSettings render_settings)
                    : unit_quad_pos_only(), white_pixel_tex(Color3{ 255, 255, 255 }),
                      fbo3d_conv_tex(init_width, init_height, GL_RGB),
                      fbo3d_rbo_color(empty_id),
                      #ifdef USE_COMBINED_FBO_BUFFERS
                        fbo3d_rbo_depth_stencil(empty_id),
                      #else
                        fbo3d_rbo_depth(empty_id),
                        fbo3d_rbo_stencil(empty_id),
                      #endif
                      fbo3d_unconv(), fbo3d_conv(), fbo3d_samples(fbo3d_samples), fbo3d_unconv_size(init_width, init_height),
                      render_settings(render_settings), render_settings_default(render_settings)
{
    //checking the constructors
    assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG
    assert(!Utils::checkForGLError());

    if (white_pixel_tex.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create FrameBuffer color texture for 3D rendering!\n");
        return;
    }

    if (fbo3d_conv_tex.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create FrameBuffer color texture for 3D scene conversion!\n");
        return;
    }

    //VBOs and Meshes
    unit_quad_pos_only = std::move(Meshes::generateQuadVBO(glm::vec2(1.f), glm::vec2(0.f), Meshes::TexcoordStyle::none, false));
    if (unit_quad_pos_only.m_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize unit quad VBO!\n");
        return;
    }

    //renderbuffers
    //TODO render buffer object abstraction
    {
        assert(!Utils::checkForGLError());
        
        #ifdef USE_COMBINED_FBO_BUFFERS
            GLuint rbos[2]{};
        #else
            GLuint rbos[3]{};
        #endif
        const size_t rbos_count = sizeof(rbos)/sizeof(rbos[0]);

        glGenRenderbuffers(rbos_count, rbos);
        if (Utils::checkForGLError())
        {
            fprintf(stderr, "Failed to create RenderBuffers(%zu) for 3D FrameBuffer!\n", rbos_count);
            return;
        }

        #ifdef USE_COMBINED_FBO_BUFFERS
            fbo3d_rbo_color = rbos[0];
            fbo3d_rbo_depth_stencil = rbos[1];
        #else
            fbo3d_rbo_color = rbos[0];
            fbo3d_rbo_depth = rbos[1];
            fbo3d_rbo_stencil = rbos[2];
        #endif
    }

    bool fbo3d_multisampled = (fbo3d_samples > 1);

    //color renderbuffer with alpha
    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_color);
    if (fbo3d_multisampled)
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, fbo3d_rbo_color_internalformat, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
    }
    else
    {
        glRenderbufferStorage(GL_RENDERBUFFER, fbo3d_rbo_color_internalformat, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
    }

    #ifdef USE_COMBINED_FBO_BUFFERS
        //depth-stencil combined renderbuffer allocation
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth_stencil);
        if (fbo3d_multisampled)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, GL_DEPTH_STENCIL, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        else
        {
            //TODO GL_DEPTH_STENCIL or GL_DEPTH24_STENCIL8?
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
    #else
        //depth renderbuffer allocation
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
        if (fbo3d_multisampled)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, GL_DEPTH_COMPONENT16, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }

        //stencil renderbuffer allocation
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
        if (fbo3d_multisampled)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, GL_STENCIL_INDEX8, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
    #endif

    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);

    //framebuffer 3D
    using FrameBuffer = Drawing::FrameBuffer;

    fbo3d_unconv.init();
    if (fbo3d_unconv.m_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize FrameBuffer for 3D scene!\n");
        return;
    }

    #ifdef USE_COMBINED_FBO_BUFFERS
        fbo3d_unconv.attachAllCombined(FrameBuffer::Attachment{ fbo3d_rbo_color, FrameBuffer::AttachmentType::render },
                                       FrameBuffer::Attachment{ fbo3d_rbo_depth_stencil, FrameBuffer::AttachmentType::render });
    #else
        fbo3d_unconv.attachAllSeparated(FrameBuffer::Attachment{ fbo3d_rbo_color, FrameBuffer::AttachmentType::render },
                                        FrameBuffer::Attachment{ fbo3d_rbo_depth, FrameBuffer::AttachmentType::render },
                                        // FrameBuffer::Attachment{ fbo3d_rbo_stencil, FrameBuffer::AttachmentType::render }
                                        FrameBuffer::Attachment{ 0, FrameBuffer::AttachmentType::none } //TODO fix stencil buffer not working on nvidia
                                       );
    #endif
    
    if (!fbo3d_unconv.isComplete())
    {
        fprintf(stderr, "FrameBuffer for 3D scene is not complete!\n");
        fbo3d_unconv.deinit();
        return;
    }

    //framebuffer 3D converted
    fbo3d_conv.init();
    if (fbo3d_conv.m_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize FrameBuffer for 3D scene conversion!\n");
        fbo3d_unconv.deinit();
        return;
    }

    fbo3d_conv.attachColorBuffer(fbo3d_conv_tex.asFrameBufferAttachment());

    if (!fbo3d_conv.isComplete())
    {
        fprintf(stderr, "FrameBuffer for 3D scene conversion is not complete!\n");
        fbo3d_unconv.deinit();
        fbo3d_conv.deinit();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, empty_id);

    assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG
}

SharedGLContext::~SharedGLContext()
{
    glDeleteBuffers(1, &fbo3d_rbo_color);
    #ifdef USE_COMBINED_FBO_BUFFERS
        glDeleteBuffers(1, &fbo3d_rbo_depth_stencil);
    #else
        glDeleteBuffers(1, &fbo3d_rbo_depth);
        glDeleteBuffers(1, &fbo3d_rbo_stencil);
    #endif
}

bool SharedGLContext::isInitialized() const
{
    return unit_quad_pos_only.m_id != empty_id &&
           white_pixel_tex.m_id != empty_id &&
           fbo3d_unconv.m_id != empty_id &&
           fbo3d_conv.m_id != empty_id &&
           fbo3d_samples >= 1;
}

glm::ivec2 SharedGLContext::getFbo3DSize(bool converted) const
{
    return converted ? glm::ivec2(fbo3d_conv_tex.m_width, fbo3d_conv_tex.m_height)
                     : fbo3d_unconv_size;
}

void SharedGLContext::changeFbo3DSize(unsigned int new_width, unsigned int new_height)
{
    const glm::ivec2 current_size = getFbo3DSize(true);

    if (new_width == current_size.x && new_height == current_size.y)
    {
        printf("Requested change of size, but FBO 3D size already is: %dx%d\n", new_width, new_height);
        return;
    }
    
    printf("Changing FBO 3D size to: %dx%d\n", new_width, new_height);

    //TODO implement check for out of memory and other OpenGL errors
    assert(!Utils::checkForGLError());

    // resize the fbo renderbuffers
    fbo3d_unconv_size = glm::ivec2(new_width, new_height);

    bool fbo3d_multisampled = (fbo3d_samples > 1);

    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_color);
    if (fbo3d_multisampled)
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, fbo3d_rbo_color_internalformat, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
    }
    else
    {
        glRenderbufferStorage(GL_RENDERBUFFER, fbo3d_rbo_color_internalformat, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
    }
    assert(!Utils::checkForGLErrorsAndPrintThem());

    #ifdef USE_COMBINED_FBO_BUFFERS
        //combined resize
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth_stencil);
        if (fbo3d_multisampled)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, GL_DEPTH_STENCIL, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        assert(!Utils::checkForGLErrorsAndPrintThem());
    #else
        //depth resize
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
        if (fbo3d_multisampled)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, GL_DEPTH_COMPONENT16, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        assert(!Utils::checkForGLErrorsAndPrintThem());

        //stencil resize
        glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
        if (fbo3d_multisampled)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, fbo3d_samples, GL_STENCIL_INDEX8, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo3d_unconv_size.x, fbo3d_unconv_size.y);
        }
        assert(!Utils::checkForGLErrorsAndPrintThem());
    #endif

    // resize the fbo converted texture
    fbo3d_conv_tex.changeTexture(new_width, new_height, GL_RGB);
    assert(!Utils::checkForGLErrorsAndPrintThem());

    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);

    assert(fbo3d_unconv.isComplete());
    assert(fbo3d_conv.isComplete());
}

bool SharedGLContext::convertFbo3D() const
{
    if (!fbo3d_unconv.isComplete() || !fbo3d_conv.isComplete()) return false;

    const glm::ivec2 fbo_src_size = getFbo3DSize(false);
    bool fbo3d_multisampled = (fbo3d_samples > 1);

    if (!fbo3d_multisampled)
    {
        fbo3d_unconv.bind();
        fbo3d_conv_tex.bind();

        glCopyTexImage2D(fbo3d_conv_tex.getBindType(), 0, GL_RGB, 0, 0, fbo_src_size.x, fbo_src_size.y, 0);

        fbo3d_unconv.unbind();
    }
    #ifdef BUILD_OPENGL_330_CORE
    else
    {
        const glm::ivec2 fbo_dst_size = getFbo3DSize(true);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo3d_unconv.m_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo3d_conv.m_id);

        glBlitFramebuffer(0, 0, fbo_src_size.x, fbo_src_size.y, 0, 0, fbo_dst_size.x, fbo_dst_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, empty_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, empty_id);
    }
    #else
    else
    {
        return false;
    }
    #endif

    return true;
}

void SharedGLContext::saveToFbo3DFromExternal(GLuint external_fbo_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, external_fbo_id);
    fbo3d_conv_tex.bind();

    const glm::ivec2 src_size = getFbo3DSize(false);
    glCopyTexImage2D(fbo3d_conv_tex.getBindType(), 0, GL_RGB, 0, 0, src_size.x, src_size.y, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, empty_id);
}

bool SharedGLContext::stageFbo3D(std::optional<GLuint> external_fbo_id_used)
{
    if (external_fbo_id_used.has_value()) // scene got rendered into external Framebuffer, save it into shared one
    {
        saveToFbo3DFromExternal(external_fbo_id_used.value());
        return true; // can't fail
    }
    else
    {
        return convertFbo3D();
    }
}

const Textures::Texture2D& SharedGLContext::getFbo3DTexture() const
{
    return fbo3d_conv_tex;
}

const Drawing::FrameBuffer& SharedGLContext::getFbo3D(bool converted) const
{
    return converted ? fbo3d_conv : fbo3d_unconv;
}

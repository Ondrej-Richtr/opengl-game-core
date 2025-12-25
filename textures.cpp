#include "game.hpp"
#include "stb_image.h"


Textures::Texture2D::Texture2D(unsigned int width, unsigned int height, GLenum component_type, unsigned int samples)
            : m_id(empty_id), m_width(width), m_height(height), m_samples(samples)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_width = 0;
        m_height = 0;
        m_samples = 0;
        return;
    }

    // bind the OpenGL texture object
    GLenum bind_type = getBindType();
    glBindTexture(bind_type, m_id);

    if (bind_type == GL_TEXTURE_2D_MULTISAMPLE)
    {
        glTexImage2DMultisample(bind_type, m_samples, component_type, m_width, m_height, GL_TRUE);
    }
    else
    {
        glTexImage2D(bind_type, 0, component_type, m_width, m_height, 0, component_type, GL_UNSIGNED_BYTE, NULL);
    }

    // set the min and max filtering
    constexpr GLint min_filtering = Utils::filteringEnumWithoutMipmap(Textures::default_min_filtering);
    glTexParameteri(bind_type, GL_TEXTURE_MIN_FILTER, min_filtering);
    glTexParameteri(bind_type, GL_TEXTURE_MAG_FILTER, Textures::default_max_filtering); // max filtering should be already without mipmaps!
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(bind_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(bind_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // unbind the texture just in case
    glBindTexture(bind_type, empty_id);
}

Textures::Texture2D::Texture2D(const char *image_path, bool generate_mipmaps)
            : m_id(empty_id), m_width(0), m_height(0), m_samples(1)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_samples = 0;
        return;
    }
    
    // load the image data using stbi
    int wanted_channels = 4, // we force 4 channels as we always want RGBA textures
        loaded_width, loaded_height, actual_channels;
    
    unsigned char *data = stbi_load(image_path, &loaded_width, &loaded_height, &actual_channels, wanted_channels);
    if (!data || loaded_width <= 0 || loaded_height <= 0)
    {
        fprintf(stderr, "Can't initialize texture - failed to load image data from '%s' with forced %d channels!\n",
                        image_path, wanted_channels);

        stbi_image_free(data); // in case the data was loaded
        m_samples = 0;
        return;
    }

    m_width = loaded_width;
    m_height = loaded_height;

    // bind the OpenGL texture object
    GLenum bind_type = getBindType();
    glBindTexture(bind_type, m_id);

    // set the texture wrapping to default values
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_S, Textures::default_wrapping);
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_T, Textures::default_wrapping);
    // set texture filtering to default values, remove mipmaps if not needed
    GLint min_filtering = generate_mipmaps ? Textures::default_min_filtering
                                           : Utils::filteringEnumWithoutMipmap(Textures::default_min_filtering);
    glTexParameteri(bind_type, GL_TEXTURE_MIN_FILTER, min_filtering);
    glTexParameteri(bind_type, GL_TEXTURE_MAG_FILTER, default_max_filtering); // max filtering should be already without mipmaps!

    // upload the image data into the texture on gpu
    glTexImage2D(bind_type, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    if (generate_mipmaps)
    {
        glGenerateMipmap(bind_type);
        assert(!Utils::checkForGLErrorsAndPrintThem()); //TODO make this an actual check + error
    }

    stbi_image_free(data);                  // we can free the image data - their copy should be on gpu
    glBindTexture(bind_type, empty_id); // unbind the texture just in case
}

Textures::Texture2D::Texture2D(const void *img_data, unsigned int width, unsigned int height, bool generate_mipmaps)
            : m_id(empty_id), m_width(width), m_height(height), m_samples(1)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_width = 0;
        m_height = 0;
        m_samples = 0;
        return;
    }

    // bind the OpenGL texture object
    GLenum bind_type = getBindType();
    glBindTexture(bind_type, m_id);

    // set the texture wrapping to default values
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_S, Textures::default_wrapping);	
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_T, Textures::default_wrapping);
    // set texture filtering to default values, remove mipmaps if not needed
    GLint min_filtering = generate_mipmaps ? Textures::default_min_filtering
                                           : Utils::filteringEnumWithoutMipmap(Textures::default_min_filtering);
    glTexParameteri(bind_type, GL_TEXTURE_MIN_FILTER, min_filtering);
    glTexParameteri(bind_type, GL_TEXTURE_MAG_FILTER, Textures::default_max_filtering); // max filtering should be already without mipmaps!

    // upload the image data into the texture on gpu
    glTexImage2D(bind_type, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

    if (generate_mipmaps)
    {
        glGenerateMipmap(bind_type);
        assert(!Utils::checkForGLErrorsAndPrintThem()); //TODO make this an actual check + error
    }

    // unbind the texture just in case
    glBindTexture(bind_type, empty_id);
}

Textures::Texture2D::Texture2D(Color3 color) // creates 1x1 texture from singular color
                : m_id(empty_id), m_width(1), m_height(1), m_samples(1)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_width = 0;
        m_height = 0;
        m_samples = 0;
        return;
    }

    changeTextureToPixel(color);
}

Textures::Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_id);
}

GLenum Textures::Texture2D::getBindType() const
{
    return isMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

void Textures::Texture2D::bind(unsigned int unit) const
{
    GLenum bind_type = getBindType();

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(bind_type, m_id);
}

bool Textures::Texture2D::isMultiSampled() const
{
    return (m_samples > 1);
}

void Textures::Texture2D::changeTexture(unsigned int new_width, unsigned int new_height,
                                        GLenum component_type, const void *new_data)
{
    m_width = new_width;
    m_height = new_height;
    // `m_samples` stays the same

    GLenum bind_type = getBindType();
    glBindTexture(bind_type, m_id);

    if (bind_type == GL_TEXTURE_2D_MULTISAMPLE)
    {
        assert(new_data == NULL);
        glTexImage2DMultisample(bind_type, m_samples, component_type, m_width, m_height, GL_TRUE);
    }
    else
    {
        glTexImage2D(bind_type, 0, component_type, m_width, m_height, 0, component_type, GL_UNSIGNED_BYTE, new_data);
    }

    //TODO unbinding is an OpenGL anti-pattern
    glBindTexture(bind_type, empty_id); // unbind the texture just in case
}

void Textures::Texture2D::changeTextureToPixel(Color3 color)
{
    assert(!isMultiSampled());

    m_width = 1;
    m_height = 1;

    // bind the OpenGL texture object
    GLenum bind_type = getBindType();
    glBindTexture(bind_type, m_id);

    // set the simplest texture wrapping and filtering
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(bind_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(bind_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(bind_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // upload the singular pixel onto gpu
    unsigned char pixel[] = { color.r, color.g, color.b };
    glTexImage2D(bind_type, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);
    
    assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG

    // unbind the texture just in case
    glBindTexture(bind_type, empty_id);
}

bool Textures::Texture2D::copyContentsFrom(const Drawing::FrameBuffer& fbo_src, unsigned int width, unsigned int height, GLenum format)
{
    if (m_id == empty_id || !fbo_src.isComplete())
    {
        return false;
    }

    fbo_src.bind();
    bind();

    //FIXME blit for opengl 3.3
    glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0, width, height, 0);

    // fbo_src.unbind();

    m_width = width;
    m_height = height;
    return true;
}

//FIXME implement or remove this
// bool Textures::Texture2D::blitFromFBO(const Drawing::FrameBuffer& fbo_src, unsigned int width, unsigned int height, GLenum format)
// {
// }

Drawing::FrameBuffer::Attachment Textures::Texture2D::asFrameBufferAttachment() const
{
    Drawing::FrameBuffer::AttachmentType attach_type = isMultiSampled() ? Drawing::FrameBuffer::AttachmentType::textureMultiSample
                                                                        : Drawing::FrameBuffer::AttachmentType::texture;
    return Drawing::FrameBuffer::Attachment{ m_id, attach_type };
}

Textures::Cubemap::~Cubemap()
{
    glDeleteTextures(1, &m_id);
}

void Textures::Cubemap::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Textures::Cubemap::createEmpty(unsigned int width_height, GLenum component_type, bool generate_mipmaps)
{
    // release the old cubemap data
    if (m_id != empty_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = empty_id;
    }
    m_size_per_face = { 0, 0, 0, 0, 0, 0 };

    // create new cubemap handle
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL handle for cubemap texture!\n");
        return;
    }

    m_size_per_face = { width_height, width_height, width_height, width_height, width_height, width_height };

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    // create face textures
    for (int i = 0; i < 6; ++i)
    {
        GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        unsigned int size = m_size_per_face[i];
        glTexImage2D(face, 0, component_type, size, size, 0, component_type, GL_UNSIGNED_BYTE, NULL);
    }

    // set cubemap filtering to default values, remove mipmaps if not needed
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, cubemap_default_wrapping);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, cubemap_default_wrapping);
    #ifdef BUILD_OPENGL_330_CORE
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, cubemap_default_wrapping);
    #endif /* BUILD_OPENGL_330_CORE */
    GLint min_filtering = generate_mipmaps ? cubemap_default_min_filtering
                                           : Utils::filteringEnumWithoutMipmap(cubemap_default_min_filtering);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, cubemap_default_max_filtering);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filtering);

    if (generate_mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        assert(!Utils::checkForGLErrorsAndPrintThem()); //TODO make this an actual check + error
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, empty_id); // unbind the cubemap just in case
}

void Textures::Cubemap::createFrom6Images(const std::array<const char*, 6>& image_paths, bool generate_mipmaps)
{
    // release the old cubemap data
    if (m_id != empty_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = empty_id;
    }
    m_size_per_face = { 0, 0, 0, 0, 0, 0 };

    // create new cubemap handle
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL handle for cubemap texture!\n");
        return;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    // create face textures from image paths
    bool load_error = false;
    int wanted_channels = 3, // skybox does not need alpha channel
        loaded_width, loaded_height, actual_channels;

    for (size_t i = 0; i < image_paths.size() && !load_error; ++i)
    {
        // load the image data using stbi
        unsigned char *data = stbi_load(image_paths[i], &loaded_width, &loaded_height, &actual_channels, wanted_channels);
        if (!data || loaded_width <= 0 || loaded_height <= 0)
        {
            fprintf(stderr, "Can't create cubemap from image paths - failed to load image data from '%s'!\n", image_paths[i]);

            load_error = true;
        }
        else if (loaded_width != loaded_height)
        {
            fprintf(stderr, "Can't create cubemap from iamge paths - image at '%s' has non-square dimensions %dx%d!\n",
                    image_paths[i], loaded_width, loaded_height);
            
            load_error = true;
        }
        else
        {
            m_size_per_face[i] = loaded_width;
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            glTexImage2D(face, 0, GL_RGB, m_size_per_face[i], m_size_per_face[i], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //TODO check for opengl errors?
        }

        stbi_image_free(data);
    }

    // set cubemap filtering to default values, remove mipmaps if not needed
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, cubemap_default_wrapping);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, cubemap_default_wrapping);
    #ifdef BUILD_OPENGL_330_CORE
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, cubemap_default_wrapping);
    #endif /* BUILD_OPENGL_330_CORE */
    GLint min_filtering = generate_mipmaps ? cubemap_default_min_filtering
                                           : Utils::filteringEnumWithoutMipmap(cubemap_default_min_filtering);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, cubemap_default_max_filtering);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filtering);

    if (load_error)
    {
        glDeleteTextures(1, &m_id);
        m_id = empty_id;
    }
    else if (generate_mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        assert(!Utils::checkForGLErrorsAndPrintThem()); //TODO make this an actual check + error
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, empty_id); // unbind the cubemap just in case
}

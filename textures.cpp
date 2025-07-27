#include "game.hpp"
#include "stb_image.h"


Textures::Texture2D::Texture2D(unsigned int width, unsigned int height, GLenum component_type)
            : m_id(empty_id), m_width(width), m_height(height)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_width = 0;
        m_height = 0;
        return;
    }

    // bind the OpenGL texture object
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexImage2D(GL_TEXTURE_2D, 0, component_type, m_width, m_height, 0, component_type, GL_UNSIGNED_BYTE, NULL);

    // set the min and max filtering
    constexpr GLint min_filtering = Utils::filteringEnumWithoutMipmap(Textures::default_min_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Textures::default_max_filtering); // max filtering should be already without mipmaps!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // unbind the texture just in case
    glBindTexture(GL_TEXTURE_2D, empty_id);
}

Textures::Texture2D::Texture2D(const char *image_path, bool generate_mipmaps)
            : m_id(empty_id), m_width(0), m_height(0)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
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

        return;
    }

    m_width = loaded_width;
    m_height = loaded_height;

    // bind the OpenGL texture object
    glBindTexture(GL_TEXTURE_2D, m_id);

    // set the texture wrapping to default values
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Textures::default_wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Textures::default_wrapping);
    // set texture filtering to default values, remove mipmaps if not needed
    GLint min_filtering = generate_mipmaps ? Textures::default_min_filtering
                                           : Utils::filteringEnumWithoutMipmap(Textures::default_min_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, default_max_filtering); // max filtering should be already without mipmaps!

    // upload the image data into the texture on gpu
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    if (generate_mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        assert(!Utils::checkForGLErrorsAndPrintThem()); //TODO make this an actual check + error
    }

    stbi_image_free(data);                  // we can free the image data - their copy should be on gpu
    glBindTexture(GL_TEXTURE_2D, empty_id); // unbind the texture just in case
}

Textures::Texture2D::Texture2D(const void *img_data, unsigned int width, unsigned int height, bool generate_mipmaps)
            : m_id(empty_id), m_width(width), m_height(height)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_width = 0;
        m_height = 0;
        return;
    }

    // bind the OpenGL texture object
    glBindTexture(GL_TEXTURE_2D, m_id);

    // set the texture wrapping to default values
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Textures::default_wrapping);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Textures::default_wrapping);
    // set texture filtering to default values, remove mipmaps if not needed
    GLint min_filtering = generate_mipmaps ? Textures::default_min_filtering
                                           : Utils::filteringEnumWithoutMipmap(Textures::default_min_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Textures::default_max_filtering); // max filtering should be already without mipmaps!

    // upload the image data into the texture on gpu
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

    if (generate_mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        assert(!Utils::checkForGLErrorsAndPrintThem()); //TODO make this an actual check + error
    }

    // unbind the texture just in case
    glBindTexture(GL_TEXTURE_2D, empty_id);
}

Textures::Texture2D::Texture2D(Color3 color) // creates 1x1 texture from singular color
                : m_id(empty_id), m_width(1), m_height(1)
{
    // generate the OpenGL texture object
    glGenTextures(1, &m_id);
    if (m_id == empty_id)
    {
        fprintf(stderr, "Failed to create OpenGL texture!\n");
        m_width = 0;
        m_height = 0;
        return;
    }

    changeTextureToPixel(color);
}

Textures::Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_id);
}

void Textures::Texture2D::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Textures::Texture2D::changeTexture(unsigned int new_width, unsigned int new_height,
                                        GLenum component_type, const void *new_data)
{
    m_width = new_width;
    m_height = new_height;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, component_type, m_width, m_height, 0, component_type, GL_UNSIGNED_BYTE, new_data);
    //TODO unbinding is an OpenGL anti-pattern
    glBindTexture(GL_TEXTURE_2D, empty_id); // unbind the texture just in case
}

void Textures::Texture2D::changeTextureToPixel(Color3 color)
{
    m_width = 1;
    m_height = 1;

    // bind the OpenGL texture object
    glBindTexture(GL_TEXTURE_2D, m_id);

    // set the simplest texture wrapping and filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // upload the singular pixel onto gpu
    unsigned char pixel[] = { color.r, color.g, color.b };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);
    
    assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG

    // unbind the texture just in case
    glBindTexture(GL_TEXTURE_2D, empty_id);
}

bool Textures::Texture2D::copyContentsFrom(const Drawing::FrameBuffer& fbo_src, unsigned int width, unsigned int height, GLenum format)
{
    if (m_id == empty_id || !fbo_src.isComplete())
    {
        return false;
    }

    fbo_src.bind();
    bind();

    glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0, width, height, 0);

    // fbo_src.unbind();

    m_width = width;
    m_height = height;
    return true;
}

Drawing::FrameBuffer::Attachment Textures::Texture2D::asFrameBufferAttachment() const
{
    return Drawing::FrameBuffer::Attachment{ m_id, Drawing::FrameBuffer::AttachmentType::texture };
}

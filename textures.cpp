#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "stb_image.h"


Textures::Texture2D::Texture2D(const char *image_path, bool generate_mipmaps)
            : m_id(Textures::empty_id), m_width(0), m_height(0)
{
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

    // generate and bind the OpenGL texture object
    glGenTextures(1, &m_id);

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
    }

    stbi_image_free(data);                              // we can free the image data - their copy should be on gpu
    glBindTexture(GL_TEXTURE_2D, Textures::empty_id);   // unbind the texture just in case
}

Textures::Texture2D::Texture2D(const void *img_data, unsigned int width, unsigned int height, bool generate_mipmaps)
            : m_id(Textures::empty_id), m_width(width), m_height(height)
{
    // generate and bind the OpenGL texture object
    glGenTextures(1, &m_id);

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
    }

    // unbind the texture just in case
    glBindTexture(GL_TEXTURE_2D, Textures::empty_id);
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

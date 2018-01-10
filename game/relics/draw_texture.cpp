
#include "stdafx.h"
#include "draw_texture.h"
#include "utils.h"


// Constructor from a texture file name.
DrawTexture::DrawTexture(const std::string &fname)
{
    m_fname = fname;
    m_texture_id = 0;

    // Load the image.
    m_image = std::make_unique<sf::Image>();
    if (!ReadImageResource(m_image.get(), fname)) {
        assert(false);
        return;
    }

    m_image->flipVertically();
    sf::Vector2u image_size = m_image->getSize();
    const sf::Uint8 *pixels = m_image->getPixelsPtr();

    // Bind the texture.
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // Right after the texture is bound, we need to set this up.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Hmm! Hard to find an answer on the exact format SFML uses.
    // I'm going to presume it's standard RGBA (8 bits per channel) until I find otherwise.
    glTexImage2D(
        GL_TEXTURE_2D, 0,              // target, level of detail
        GL_RGBA8,                      // internal format
        image_size.x, image_size.y, 0, // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,     // external format, type
        pixels);                       // pixels

    // Then, generate mimaps.
    glGenerateMipmap(GL_TEXTURE_2D);

    // All done.
    glBindTexture(GL_TEXTURE_2D, 0);
}


// Destructor. Free up the texture.
DrawTexture::~DrawTexture()
{
    if (m_texture_id != 0) {
        glDeleteTextures(1, &m_texture_id);
    }

    m_fname = "";
    m_texture_id = 0;
}

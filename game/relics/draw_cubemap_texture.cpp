
#include "stdafx.h"
#include "draw_cubemap_texture.h"
#include "common_util.h"

#include "utils.h"


DrawCubemapTexture::DrawCubemapTexture(
    const std::string &fname_north,
    const std::string &fname_south,
    const std::string &fname_east,
    const std::string &fname_west,
    const std::string &fname_top,
    const std::string &fname_bottom) :
    m_fname_north(fname_north),
    m_fname_south(fname_south),
    m_fname_east(fname_east),
    m_fname_west(fname_west),
    m_fname_top(fname_top),
    m_fname_bottom(fname_bottom)
{
    // Load our six image.
    sf::Image image_north;
    if (!ReadImageResource(&image_north, fname_north)) {
        return;
    }

    sf::Image image_south;
    if (!ReadImageResource(&image_south, fname_south)) {
        return;
    }

    sf::Image image_east;
    if (!ReadImageResource(&image_east, fname_east)) {
        return;
    }

    sf::Image image_west;
    if (!ReadImageResource(&image_west, fname_west)) {
        return;
    }

    sf::Image image_top;
    if (!ReadImageResource(&image_top, fname_top)) {
        return;
    }

    sf::Image image_bottom;
    if (!ReadImageResource(&image_bottom, fname_bottom)) {
        return;
    }
    
    // Flip all the images vertically.
#if 0
    image_north.flipVertically();
    image_south.flipVertically();
    image_east.flipVertically();
    image_west.flipVertically();
    image_top.flipVertically();
    image_bottom.flipVertically();
#endif

    // Make sure all the images are the same size.
    sf::Vector2u size_north  = image_north.getSize();
    sf::Vector2u size_south  = image_south.getSize();
    sf::Vector2u size_east   = image_east.getSize();
    sf::Vector2u size_west   = image_west.getSize();
    sf::Vector2u size_top    = image_top.getSize();
    sf::Vector2u size_bottom = image_bottom.getSize();

    if (size_north != size_south) {
        PrintDebug(
            "South face %s has differing size: [%d, %d] vs. [%d, %d]\n",
            m_fname_south.c_str(), size_south.x, size_south.y, size_north.x, size_north.y);
        return;
    }

    if (size_north != size_east) {
        PrintDebug(
            "East face %s has differing size: [%d, %d] vs. [%d, %d]\n",
            m_fname_east.c_str(), size_east.x, size_east.y, size_north.x, size_north.y);
        return;
    }

    if (size_north != size_west) {
        PrintDebug(
            "West face %s has differing size: [%d, %d] vs. [%d, %d]\n",
            m_fname_west.c_str(), size_west.x, size_west.y, size_north.x, size_north.y);
        return;
    }

    if (size_north != size_top) {
        PrintDebug(
            "Top face %s has differing size: [%d, %d] vs. [%d, %d]\n",
            m_fname_top.c_str(), size_top.x, size_top.y, size_north.x, size_north.y);
        return;
    }

    if (size_north != size_bottom) {
        PrintDebug(
            "Bottom face %s has differing size: [%d, %d] vs. [%d, %d]\n",
            m_fname_bottom.c_str(), size_bottom.x, size_bottom.y, size_north.x, size_north.y);
        return;
    }

    // Bind the texture.
    glGenTextures(1, &m_cubemap_texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture_id);

    // Load the six faces. Hard to find an answer on the exact format SFML uses.
    // I'm going to presume it's standard RGBA (8 bits per channel) until I find otherwise.
    const sf::Uint8 *pixels_north  = image_north.getPixelsPtr();
    const sf::Uint8 *pixels_south  = image_south.getPixelsPtr();
    const sf::Uint8 *pixels_east   = image_east.getPixelsPtr();
    const sf::Uint8 *pixels_west   = image_west.getPixelsPtr();
    const sf::Uint8 *pixels_top    = image_top.getPixelsPtr();
    const sf::Uint8 *pixels_bottom = image_bottom.getPixelsPtr();

    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,  // target, level of detail
        0, GL_RGBA8,                     // level of detail, internal format
        size_north.x, size_north.y, 0,   // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,       // external format, type
        pixels_north);                   // pixels

    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,  // target, level of detail
        0, GL_RGBA8,                     // level of detail, internal format
        size_south.x, size_south.y, 0,   // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,       // external format, type
        pixels_south);                   // pixels

    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,  // target, level of detail
        0, GL_RGBA8,                     // level of detail, internal format
        size_west.x, size_west.y, 0,     // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,       // external format, type
        pixels_west);                    // pixels

    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,  // target, level of detail
        0, GL_RGBA8,                     // level of detail, internal format
        size_east.x, size_east.y, 0,     // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,       // external format, type
        pixels_east);                    // pixels

    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,  // target, level of detail
        0, GL_RGBA8,                     // level of detail, internal format
        size_bottom.x, size_bottom.y, 0, // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,       // external format, type
        pixels_bottom);                  // pixels

    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,  // target, level of detail
        0, GL_RGBA8,                     // level of detail, internal format
        size_top.x, size_top.y, 0,       // width, height, border
        GL_RGBA, GL_UNSIGNED_BYTE,       // external format, type
        pixels_top);                     // pixels

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // All done.
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


// Destructor. Free up the texture.
DrawCubemapTexture::~DrawCubemapTexture()
{
    if (m_cubemap_texture_id != 0) {
        glDeleteTextures(1, &m_cubemap_texture_id);
    }

    m_fname_north  = "";
    m_fname_south  = "";
    m_fname_east   = "";
    m_fname_west   = "";
    m_fname_top    = "";
    m_fname_bottom = "";

    m_cubemap_texture_id = 0;
}

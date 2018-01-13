#pragma once

#include "stdafx.h"


class DrawCubemapTexture
{
public:
    static std::unique_ptr<DrawCubemapTexture> Create(
        const std::string &fname_north,
        const std::string &fname_south,
        const std::string &fname_east,
        const std::string &fname_west,
        const std::string &fname_top,
        const std::string &fname_bottom);

    virtual ~DrawCubemapTexture();

    const std::string &getFileNameNorth()  const { return m_fname_north; }
    const std::string &getFileNameSouth()  const { return m_fname_south; }
    const std::string &getFileNameEast()   const { return m_fname_east; }
    const std::string &getFileNameWest()   const { return m_fname_west; }
    const std::string &getFileNameTop()    const { return m_fname_top; }
    const std::string &getFileNameBottom() const { return m_fname_bottom; }

    GLuint getTextureId() const { return m_cubemap_texture_id; }
    
private:
    FORBID_DEFAULT_CTOR(DrawCubemapTexture)
    FORBID_COPYING(DrawCubemapTexture)
    FORBID_MOVING(DrawCubemapTexture)

    // Private ctor, since Create does the work.
    DrawCubemapTexture(
        const std::string &fname_north,  sf::Image *image_north,
        const std::string &fname_south,  sf::Image *image_south,
        const std::string &fname_east,   sf::Image *image_east,
        const std::string &fname_west,   sf::Image *image_west,
        const std::string &fname_top,    sf::Image *image_top,
        const std::string &fname_bottom, sf::Image *image_bottom);

    // Private data.
    std::string m_fname_north;
    std::string m_fname_south;
    std::string m_fname_east;
    std::string m_fname_west;
    std::string m_fname_top;
    std::string m_fname_bottom;

    GLuint m_cubemap_texture_id;
};


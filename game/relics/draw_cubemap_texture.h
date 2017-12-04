#pragma once

#include "stdafx.h"


class DrawCubemapTexture
{
public:
    DrawCubemapTexture(
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
    // Forbid copies, and default ctor.
    DrawCubemapTexture() = delete;
    DrawCubemapTexture(const DrawCubemapTexture &that) = delete;
    void operator=(const DrawCubemapTexture &that) = delete;

    std::string m_fname_north;
    std::string m_fname_south;
    std::string m_fname_east;
    std::string m_fname_west;
    std::string m_fname_top;
    std::string m_fname_bottom;

    GLuint m_cubemap_texture_id;
};


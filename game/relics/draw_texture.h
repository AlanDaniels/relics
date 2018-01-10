#pragma once

class DrawTexture
{
public:
    DrawTexture(const std::string &fname);
    virtual ~DrawTexture();

    const std::string &getFileName() const { return m_fname; }
    GLuint getTextureId() const { return m_texture_id;  }

private:
    DISALLOW_DEFAULT(DrawTexture)
    DISALLOW_COPYING(DrawTexture)
    DISALLOW_MOVING(DrawTexture)

    // Private data
    std::string m_fname;
    std::unique_ptr<sf::Image> m_image;
    GLuint m_texture_id;
};

#pragma once

class DrawTexture
{
public:
    static std::unique_ptr<DrawTexture> Create(const std::string &fname);

    virtual ~DrawTexture();

    const std::string &getPath() const { return m_filename; }
    GLuint getTextureId() const { return m_texture_id; }

private:
    DISALLOW_DEFAULT(DrawTexture)
    DISALLOW_COPYING(DrawTexture)
    DISALLOW_MOVING(DrawTexture)

    // Private ctor, since Create does the work.
    DrawTexture(const std::string &fname, sf::Image *image);

    // Private data
    std::string m_filename;
    GLuint m_texture_id;
};

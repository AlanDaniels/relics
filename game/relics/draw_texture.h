#pragma once

class DrawTexture
{
public:
    DrawTexture(const std::string &fname);
    virtual ~DrawTexture();

    const std::string &getFileName() const { return m_fname; }
    GLuint getTextureId() const { return m_texture_id;  }

private:
    // Forbid copies, and default ctor.
    DrawTexture() = delete;
    DrawTexture(const DrawTexture &that) = delete;
    void operator=(const DrawTexture &that) = delete;

    std::string m_fname;
    GLuint m_texture_id;
};

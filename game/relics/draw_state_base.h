#pragma once

#include "stdafx.h"
#include "my_math.h"


class DrawCubemapTexture;
class DrawTexture;


// Keep track of all our draw state settings in one place.
struct DrawStateSettings
{
    DrawStateSettings() :
        title(""),
        enable_blending(true),
        enable_depth_test(true),
        depth_func(GL_LEQUAL),
        draw_mode(GL_TRIANGLES),
        vert_shader_fname(""),
        frag_shader_fname("") {}

    ~DrawStateSettings() {}

    std::string title;
    bool        enable_blending;
    bool        enable_depth_test;
    GLenum      depth_func;
    GLenum      draw_mode;
    std::string vert_shader_fname;
    std::string frag_shader_fname;
};


class DrawState_Base
{
    // I really doubt we'll ever need more than four textures.
    static const int MAX_TEXTURES = 4;

public:
    DrawState_Base(int uni_tex_count) :
        m_initialized(false),
        m_uni_tex_count(uni_tex_count),
        m_program_ID(0),
        m_vertex_shader_ID(0),
        m_fragment_shader_ID(0) {
        assert(m_uni_tex_count <= MAX_TEXTURES);
    }

    // ...but when we delete, free everything up.
    virtual ~DrawState_Base();

    bool addUniformFloat(const std::string &name);
    bool addUniformVec4(const std::string &name);
    bool addUniformMatrix4by4(const std::string &name);

    bool updateUniformFloat(const std::string &name, GLfloat val);
    bool updateUniformVec4(const std::string &name, const MyVec4 &val);
    bool updateUniformMatrix4by4(const std::string &name, const MyMatrix4by4 &val);

    bool updateUniformTexture(int index, const DrawTexture &texture);
    bool updateUniformCubemapTexture(int index, const DrawCubemapTexture &cubemap_texture);

protected:
    bool  renderSetup();
    bool  renderTeardown();
    GLint getAttribute(const std::string &name) const;

    bool create(const std::vector<std::string> &attrib_names, const DrawStateSettings &settings);

    DrawStateSettings m_settings;

private:
    FORBID_DEFAULT_CTOR(DrawState_Base)
    FORBID_COPYING(DrawState_Base)
    FORBID_MOVING(DrawState_Base)

    // Private data.
    bool m_initialized;

    GLuint m_program_ID;
    GLuint m_vertex_shader_ID;
    GLuint m_fragment_shader_ID;

    std::map<std::string, GLint> m_attrib_map;
    std::map<std::string, GLint> m_uniform_float_map;
    std::map<std::string, GLint> m_uniform_vec4_map;
    std::map<std::string, GLint> m_uniform_matrix4by4_map;

    int m_uni_tex_count;
    std::vector<GLuint> m_uniform_textures;
};

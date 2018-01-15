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

    DEFAULT_COPYING(DrawStateSettings)
    DEFAULT_MOVING(DrawStateSettings)

    std::string title;
    bool        enable_blending;
    bool        enable_depth_test;
    GLenum      depth_func;
    GLenum      draw_mode;
    std::string vert_shader_fname;
    std::string frag_shader_fname;
};


// Throughout our draw states, we'll have plenty of methods that change OpenGL's'
// actual state machine, but the internals of the object itself will be unchanged.'
// For the sake of clean code, we'll mark all these methods as "const".
// Also, I really doubt we'll ever need more than four textures.
class DrawState_Base
{
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

    virtual ~DrawState_Base();

    bool addUniformFloat(const std::string &name);
    bool addUniformVec4(const std::string &name);
    bool addUniformMatrix4by4(const std::string &name);

    bool updateUniformFloat(const std::string &name, GLfloat val) const;
    bool updateUniformVec4(const std::string &name, const MyVec4 &val) const;
    bool updateUniformMatrix4by4(const std::string &name, const MyMatrix4by4 &val) const;

    bool updateUniformTexture(int index, const DrawTexture &texture) const;
    bool updateUniformCubemapTexture(int index, const DrawCubemapTexture &cubemap_texture) const;

protected:
    bool  renderSetup() const;
    bool  renderTeardown() const;
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

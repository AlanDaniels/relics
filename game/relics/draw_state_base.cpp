
#include "stdafx.h"
#include "draw_state_base.h"
#include "common_util.h"

#include "draw_cubemap_texture.h"
#include "draw_texture.h"
#include "config.h"
#include "format.h"
#include "my_math.h"
#include "utils.h"


// Show info log data from Opengl itself.
static void ShowInfoLog(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint len;
    glGet__iv(object, GL_INFO_LOG_LENGTH, &len);
    std::unique_ptr<char[]> msg(new char[len + 1]);
    msg[len] = '\0';

    glGet__InfoLog(object, len, NULL, msg.get());
    PrintDebug(msg.get());
}


// Compile a shader.
static GLuint CompileShader(GLenum shader_type, const std::string &fname)
{
    int len;
    std::string text = ReadTextResource(fname);
    if (text == "") {
        assert(false);
        return 0;
    }

    GLuint shader = glCreateShader(shader_type);

    const char *raw_text = text.c_str();
    glShaderSource(shader, 1, &raw_text, &len);
    glCompileShader(shader);

    GLint shader_ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (shader_ok == GL_FALSE) {
        PrintDebug(fmt::format("Failed to compile shader {}...\n", fname));
        ShowInfoLog(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        assert(false);
        return 0;
    }

    return shader;
}


// Compile an entire program.
static GLuint CompileProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Check that the linking worked.
    GLint program_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (program_ok == GL_FALSE) {
        PrintDebug("Failed to link shader program...\n");
        ShowInfoLog(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        assert(false);
        return 0;
    }

    // Linking worked.
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    return program;
}


// Destructor. Clean everything up.
DrawState_Base::~DrawState_Base()
{
    // DEBUG: CLEAN EVERYTHING UP.
}


// Add a uniform float.
bool DrawState_Base::addUniformFloat(const std::string &name)
{
    if (m_initialized) {
        PrintDebug(fmt::format("Already initialized! Can't add uniform float {}.\n", name));
        return false;
    }

    const auto &iter = m_uniform_float_map.find(name);
    if (iter != m_uniform_float_map.end()) {
        PrintDebug(fmt::format("Tried to add uniform float {} twice.\n", name));
        return false;
    }

    m_uniform_float_map.emplace(name, 0);
    return true;
}


// Add a uniform vec4.
bool DrawState_Base::addUniformVec4(const std::string &name)
{
    if (m_initialized) {
        PrintDebug(fmt::format("Already initialized! Can't add uniform vec4 {}.\n", name));
        return false;
    }

    const auto &iter = m_uniform_vec4_map.find(name);
    if (iter != m_uniform_vec4_map.end()) {
        PrintDebug(fmt::format("Tried to add uniform vec4 {} twice.\n", name));
        return false;
    }

    m_uniform_vec4_map.emplace(name, 0);
    return true;
}


// Add a uniform matrix, 4-by-4.
bool DrawState_Base::addUniformMatrix4by4(const std::string &name)
{
    if (m_initialized) {
        PrintDebug(fmt::format("Already initialized! Can't add uniform matrix4by4 {}.\n", name));
        return false;
    }

    const auto &iter = m_uniform_matrix4by4_map.find(name);
    if (iter != m_uniform_matrix4by4_map.end()) {
        PrintDebug(fmt::format("Tried to add uniform matrix4by4 {} twice.\n", name));
        return false;
    }

    m_uniform_matrix4by4_map.emplace(name, 0);
    return true;
}


// Look up an attribute.
GLint DrawState_Base::getAttribute(const std::string &name) const {
    const auto &iter = m_attrib_map.find(name);
    assert(iter != m_attrib_map.end());
    return iter->second;
}


// Create all of our resoures.
bool DrawState_Base::create(
    const std::vector<std::string> &attrib_names, const DrawStateSettings &settings)
{
    if (m_initialized) {
        PrintDebug("Can't create the program twice!\n");
        return false;
    }

    // Save our settings.
    m_settings = settings;

    // Compile our shaders.
    m_vertex_shader_ID = CompileShader(GL_VERTEX_SHADER, m_settings.vert_shader_fname.c_str());
    if (m_vertex_shader_ID == 0) {
        return false;
    }

    m_fragment_shader_ID = CompileShader(GL_FRAGMENT_SHADER, m_settings.frag_shader_fname.c_str());
    if (m_fragment_shader_ID == 0) {
        return false;
    }

    m_program_ID = CompileProgram(m_vertex_shader_ID, m_fragment_shader_ID);
    if (m_program_ID == 0) {
        return false;
    }

    // Look up our uniform locations.
    // If "glGetUniformLocation" returns -1, the name was invalid.
    for (const auto &iter : m_uniform_float_map) {
        const std::string &name = iter.first;
        GLint location = glGetUniformLocation(m_program_ID, name.c_str());
        assert(location != -1);
        // TODO: Does this work?
        m_uniform_float_map[name] = location;
    }

    for (const auto &iter : m_uniform_vec4_map) {
        const std::string &name = iter.first;
        GLint location = glGetUniformLocation(m_program_ID, name.c_str());
        assert(location != -1);
        m_uniform_vec4_map[name] = location;
    }

    for (const auto &iter : m_uniform_matrix4by4_map) {
        const std::string &name = iter.first;
        GLint location = glGetUniformLocation(m_program_ID, name.c_str());
        assert(location != -1);
        m_uniform_matrix4by4_map[name] = location;
    }

    // Look up our uniform texture locations.
    // If "glGetUniformLocation" returns -1, the name was invalid.
    for (int i = 0; i < m_uni_tex_count; i++) {
        std::string tex_name = fmt::format("textures[{}]", i);
        GLint location = glGetUniformLocation(m_program_ID, tex_name.c_str());
        assert(location != -1);
        m_uniform_textures.emplace_back(location);
    }

    // Look up our attributes.
    // If "glGetAttribLocation" returns -1, the name was invalid.
    for (const auto &iter : attrib_names) {
        const std::string &name = iter;
        GLint location = glGetAttribLocation(m_program_ID, name.c_str());
        assert(location != -1);
        m_attrib_map[name] = location;
    }

    // Everything worked. Print our details.
    if (GetConfig().debug.print_draw_state) {
        PrintDebug(fmt::format("Draw state IDs for '{}'...\n", m_settings.title));
        PrintDebug(fmt::format("    Vertex Shader: {}\n", m_vertex_shader_ID));
        PrintDebug(fmt::format("    Fragment Shader: {}\n", m_fragment_shader_ID));
        PrintDebug(fmt::format("    Program: {}\n", m_program_ID));

        for (const auto &iter : m_attrib_map) {
            PrintDebug(fmt::format("    Attribute - {0}: {1}\n", iter.first, iter.second));
        }

        for (const auto &iter : m_uniform_float_map) {
            PrintDebug(fmt::format("    Uniform Float - '{0}': {1}\n", iter.first, iter.second));
        }

        for (const auto &iter : m_uniform_vec4_map) {
            PrintDebug(fmt::format("    Uniform Vec4 - '{0}': {1}\n", iter.first, iter.second));
        }

        for (const auto &iter : m_uniform_matrix4by4_map) {
            PrintDebug(fmt::format("    Uniform Matrix4by4 - '{0}': {1}\n", iter.first, iter.second));
        }

        for (int i = 0; i < m_uni_tex_count; i++) {
            PrintDebug(fmt::format("    Uniform Texture - #{0}: {1}\n", i, m_uniform_textures[i]));
        }
    }

    // All done.
    m_initialized = true;
    return true;
}


// Update a uniform float.
bool DrawState_Base::updateUniformFloat(const std::string &name, GLfloat value) const
{
    // Use our compiled program.
    glUseProgram(m_program_ID);

    const auto &iter = m_uniform_float_map.find(name);
    if (iter == m_uniform_float_map.end()) {
        PrintDebug(fmt::format("Could not find uniform float {}\n", name));
        return false;
    }

    glUniform1f(iter->second, value);
    return true;
}


// Update a uniform vec4.
bool DrawState_Base::updateUniformVec4(const std::string &name, const MyVec4 &val) const
{
    // Use our compiled program.
    glUseProgram(m_program_ID);

    const auto &iter = m_uniform_vec4_map.find(name);
    if (iter == m_uniform_vec4_map.end()) {
        PrintDebug(fmt::format("Could not find uniform vec4 {}\n", name));
        return false;
    }

    glUniform4f(iter->second, val.x(), val.y(), val.z(), val.w());
    return true;
}


// Update a uniform matrix-4by4.
bool DrawState_Base::updateUniformMatrix4by4(const std::string &name, const MyMatrix4by4 &val) const
{
    // Use our compiled program.
    glUseProgram(m_program_ID);

    const auto &iter = m_uniform_matrix4by4_map.find(name);
    if (iter == m_uniform_matrix4by4_map.end()) {
        PrintDebug(fmt::format("Could not find uniform matrix-4by4 {}\n", name));
        return false;
    }

    glUniformMatrix4fv(iter->second, 1, GL_FALSE, val.ptr());
    return true;
}


// Update a uniform texture.
bool DrawState_Base::updateUniformTexture(int index, const DrawTexture &texture) const
{
    assert(index <= MAX_TEXTURES);

    // Use our compiled program.
    glUseProgram(m_program_ID);

    GLuint texture_id = texture.getTextureId();
    assert(texture_id != 0);

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(m_uniform_textures[index], index);
    return true;
}


// Update a uniform texture, cubemap version.
bool DrawState_Base::updateUniformCubemapTexture(int index, const DrawCubemapTexture &cubemap_texture) const
{
    assert(index <= MAX_TEXTURES);

    // Use our compiled program.
    glUseProgram(m_program_ID);

    GLuint texture_id = cubemap_texture.getTextureId();
    assert(texture_id != 0);

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    glUniform1i(m_uniform_textures[index], index);
    return true;
}


// Stuff right before the render.
bool DrawState_Base::renderSetup() const
{
    // Switch to our draw state settings.
    (GetConfig().render.cull_backfaces ? glEnable : glDisable)(GL_CULL_FACE);
    (m_settings.enable_depth_test ? glEnable : glDisable)(GL_DEPTH_TEST);
    (m_settings.enable_blending   ? glEnable : glDisable)(GL_BLEND);
    glDepthFunc(m_settings.depth_func);

    // Use our compiled program.
    glUseProgram(m_program_ID);
    return true;
}


// Right after the render. Clean up after ourselves.
bool DrawState_Base::renderTeardown() const
{
    for (const auto &iter : m_attrib_map) {
        glDisableVertexAttribArray(iter.second);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

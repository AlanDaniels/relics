
#include "stdafx.h"
#include "draw_state_base.h"

#include "draw_cubemap_texture.h"
#include "draw_texture.h"
#include "config.h"
#include "my_math.h"
#include "utils.h"


// TODO: One day, go through all this and make sure you're freeing stuff up correctly.


// Show info log data from Opengl itself.
static void ShowInfoLog(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint len;
    glGet__iv(object, GL_INFO_LOG_LENGTH, &len);
    char * msg = new char[len + 1];
    msg[len] = '\0';

    glGet__InfoLog(object, len, NULL, msg);
    PrintDebug(msg);
    delete[] msg;
}


// Compile a shader.
static GLuint CompileShader(GLenum shader_type, const char *fname)
{
    int len;
    std::string text = ReadTextResource(fname);
    if (text == "") {
        return 0;
    }

    GLuint shader = glCreateShader(shader_type);
    const char *raw_text = text.c_str();
    glShaderSource(shader, 1, &raw_text, &len);
    glCompileShader(shader);

    GLint shader_ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        PrintDebug("Failed to compile %s...\n", fname);
        ShowInfoLog(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
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
    GLint program_ok;
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        PrintDebug("Failed to link shader program...\n");
        ShowInfoLog(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
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
bool DrawState_Base::addUniformFloat(const char *name)
{
    std::string key = name;

    if (m_initialized) {
        PrintDebug("Already initialized! Can't add uniform float %s.\n", key.c_str());
        return false;
    }

    auto iter = m_uniform_float_map.find(key);
    if (iter != m_uniform_float_map.end()) {
        PrintDebug("Tried to add uniform float %s twice.\n", key.c_str());
        return false;
    }

    m_uniform_float_map[key] = 0;
    return true;
}


// Add a uniform vec4.
bool DrawState_Base::addUniformVec4(const char *name)
{
    std::string key = name;

    if (m_initialized) {
        PrintDebug("Already initialized! Can't add uniform vec4 %s.\n", key.c_str());
        return false;
    }

    auto iter = m_uniform_vec4_map.find(key);
    if (iter != m_uniform_vec4_map.end()) {
        PrintDebug("Tried to add uniform vec4 %s twice.\n", key.c_str());
        return false;
    }

    m_uniform_vec4_map[key] = 0;
    return true;
}


// Add a uniform matrix, 4-by-4.
bool DrawState_Base::addUniformMatrix4by4(const char *name)
{
    std::string key = name;

    if (m_initialized) {
        PrintDebug("Already initialized! Can't add uniform matrix4by4 %s.\n", key.c_str());
        return false;
    }

    auto iter = m_uniform_matrix4by4_map.find(key);
    if (iter != m_uniform_matrix4by4_map.end()) {
        PrintDebug("Tried to add uniform matrix4by4 %s twice.\n", key.c_str());
        return false;
    }

    m_uniform_matrix4by4_map[key] = 0;
    return true;
}


// Look up an attribute.
GLint DrawState_Base::getAttribute(const char *name) const {
    std::string key = name;
    const auto &iter = m_attrib_map.find(key);
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
        const std::string &key = iter.first;
        GLint location = glGetUniformLocation(m_program_ID, key.c_str());
        assert(location != -1);
        m_uniform_float_map[key] = location;
    }

    for (const auto &iter : m_uniform_vec4_map) {
        const std::string &key = iter.first;
        GLint location = glGetUniformLocation(m_program_ID, key.c_str());
        assert(location != -1);
        m_uniform_vec4_map[key] = location;
    }

    for (const auto &iter : m_uniform_matrix4by4_map) {
        const std::string &key = iter.first;
        GLint location = glGetUniformLocation(m_program_ID, key.c_str());
        assert(location != -1);
        m_uniform_matrix4by4_map[key] = location;
    }

    // Look up our uniform texture locations.
    // If "glGetUniformLocation" returns -1, the name was invalid.
    for (int i = 0; i < m_uniform_texture_count; i++) {
        char tex_name[64];
        sprintf(tex_name, "textures[%d]", i);
        GLint location = glGetUniformLocation(m_program_ID, tex_name);
        assert(location != -1);
        m_uniform_textures.emplace_back(location);
    }

    // Look up our attributes.
    // If "glGetAttribLocation" returns -1, the name was invalid.
    for (const auto &iter : attrib_names) {
        const std::string &key = iter;
        GLint location = glGetAttribLocation(m_program_ID, key.c_str());
        assert(location != -1);
        m_attrib_map[key] = location;
    }

    // Everything worked. Print our details.
    if (GetConfig().debug.print_draw_state) {
        PrintDebug("Draw state IDs for \"%s\"...\n", m_settings.title.c_str());
        PrintDebug("    Vertex Shader: %u\n", m_vertex_shader_ID);
        PrintDebug("    Fragment Shader: %u\n", m_fragment_shader_ID);
        PrintDebug("    Program: %u\n", m_program_ID);

        for (const auto &iter : m_attrib_map) {
            PrintDebug("    Attribute - %s: %u\n", iter.first.c_str(), iter.second);
        }

        for (const auto &iter : m_uniform_float_map) {
            PrintDebug("    Uniform Float - '%s': %u\n", iter.first.c_str(), iter.second);
        }

        for (const auto &iter : m_uniform_vec4_map) {
            PrintDebug("    Uniform Vec4 - '%s': %u\n", iter.first.c_str(), iter.second);
        }

        for (const auto &iter : m_uniform_matrix4by4_map) {
            PrintDebug("    Uniform Matrix4by4 - '%s': %u\n", iter.first.c_str(), iter.second);
        }

        for (int i = 0; i < m_uniform_texture_count; i++) {
            PrintDebug("    Uniform Texture - #%d: %u\n", i, m_uniform_textures[i]);
        }
    }

    // All done.
    m_initialized = true;
    return true;
}


// Update a uniform float.
bool DrawState_Base::updateUniformFloat(const char *name, GLfloat value)
{
    // Use our compiled program.
    glUseProgram(m_program_ID);

    std::string key = name;
    auto iter = m_uniform_float_map.find(key);
    if (iter == m_uniform_float_map.end()) {
        PrintDebug("Could not find uniform float %s\n", key.c_str());
        return false;
    }

    glUniform1f(iter->second, value);
    return true;
}


// Update a uniform vec4.
bool DrawState_Base::updateUniformVec4(const char *name, const MyVec4 &val)
{
    // Use our compiled program.
    glUseProgram(m_program_ID);

    std::string key = name;
    auto iter = m_uniform_vec4_map.find(key);
    if (iter == m_uniform_vec4_map.end()) {
        PrintDebug("Could not find uniform vec4 %s\n", key.c_str());
        return false;
    }

    glUniform4f(iter->second, val.x(), val.y(), val.z(), val.w());
    return true;
}


// Update a uniform matrix-4by4.
bool DrawState_Base::updateUniformMatrix4by4(const char *name, const MyMatrix4by4 &val)
{
    // Use our compiled program.
    glUseProgram(m_program_ID);

    std::string key = name;
    auto iter = m_uniform_matrix4by4_map.find(key);
    if (iter == m_uniform_matrix4by4_map.end()) {
        PrintDebug("Could not find uniform matrix-4by4 %s\n", key.c_str());
        return false;
    }

    glUniformMatrix4fv(iter->second, 1, GL_FALSE, val.ptr());
    return true;
}


// Update a uniform texture.
bool DrawState_Base::updateUniformTexture(int index, const DrawTexture &texture)
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
bool DrawState_Base::updateUniformCubemapTexture(int index, const DrawCubemapTexture &cubemap_texture)
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
bool DrawState_Base::renderSetup()
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
bool DrawState_Base::renderTeardown()
{
    for (const auto &iter : m_attrib_map) {
        glDisableVertexAttribArray(iter.second);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return true;
}

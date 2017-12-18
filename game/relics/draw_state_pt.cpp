
#include "stdafx.h"
#include "draw_state_pt.h"

#include "utils.h"


// Create the draw state. Note our uniform attribute names are always the same.
bool DrawState_PT::create(const DrawStateSettings &settings)
{
    std::vector<std::string> attribs = { "in_position", "in_texuv" };
    return DrawState_Base::create(attribs, settings);
}


// This is where the rubber hits the road.
bool DrawState_PT::render(const VertList_PT  &vert_list)
{
    assert(vert_list.isCurrent());

    // Set up our textures.
    if (!renderSetup()) {
        return false;
    }

    // Bind the texture array the program will draw.
    glBindBuffer(GL_ARRAY_BUFFER, vert_list.getVertexBufferID());

    GLint attrib_position = getAttribute("in_position");
    GLint attrib_texuv    = getAttribute("in_texuv");

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(
        attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT),
        (void*)offsetof(Vertex_PT, position));

    glEnableVertexAttribArray(attrib_texuv);
    glVertexAttribPointer(
        attrib_texuv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT),
        (void*)offsetof(Vertex_PT, texuv));

    // And away we go.
    glDrawArrays(m_settings.draw_mode, 0, vert_list.getByteCount());

    // Clean up after ourselves.
    if (!renderTeardown()) {
        return false;
    }

    return true;
}
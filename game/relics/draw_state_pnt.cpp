
#include "stdafx.h"
#include "draw_state_pnt.h"

#include "game_world.h"
#include "utils.h"


// Create the draw state. Note our uniform attribute names are always the same.
bool DrawState_PNT::create(const DrawStateSettings &settings)
{
    std::vector<std::string> attribs = { "in_position", "in_normal", "in_texuv" };
    return DrawState_Base::create(attribs, settings);
}


// This is where the rubber hits the road.
bool DrawState_PNT::render(const VertList_PNT &vert_list)
{
    // Make sure the vert list is realized.
    assert(vert_list.isRealized());

    // Set up our textures.
    if (!renderSetup()) {
        return false;
    }

    // Bind the texture array the program will draw.
    glBindBuffer(GL_ARRAY_BUFFER, vert_list.getVertexBufferID());

    GLint attrib_position = getAttribute("in_position");
    GLint attrib_normal   = getAttribute("in_normal");
    GLint attrib_texuv    = getAttribute("in_texuv");

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(
        attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_PNT),
        (void*) offsetof(Vertex_PNT, position));

    glEnableVertexAttribArray(attrib_normal);
    glVertexAttribPointer(
        attrib_normal, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_PNT),
        (void*) offsetof(Vertex_PNT, normal));

    glEnableVertexAttribArray(attrib_texuv);
    glVertexAttribPointer(
        attrib_texuv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PNT),
        (void*) offsetof(Vertex_PNT, texuv));

    // And away we go.
    glDrawArrays(m_settings.draw_mode, 0, vert_list.getByteCount());

    // Clean up after ourselves.
    if (!renderTeardown()) {
        return false;
    }

    return true;
}
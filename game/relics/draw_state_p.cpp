
#include "stdafx.h"
#include "draw_state_p.h"
#include "utils.h"


// Create the draw state. Note our uniform attribute names are always the same.
bool DrawState_P::create(const DrawStateSettings &settings)
{
    std::vector<std::string> attribs = { "in_position", };
    return DrawState_Base::create(attribs, settings);
}


// This is where the rubber hits the road.
bool DrawState_P::render(const VertList_P  &vert_list)
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

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(
        attrib_position, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_P),
        (void*)offsetof(Vertex_P, position));

    // And away we go.
    glDrawArrays(m_settings.draw_mode, 0, vert_list.getByteCount());

    // Clean up after ourselves.
    if (!renderTeardown()) {
        return false;
    }

    return true;
}

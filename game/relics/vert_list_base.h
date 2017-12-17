#pragma once

#include "stdafx.h"


// A call to "clear" can reset these, but otherwise, we'll alway rebuild from scratch.
// Since this is a template, *all* of the code has to be in a header file.
template<typename T>
class VertList_Base
{
public:
    VertList_Base(int initial_size) :
        m_vertex_buffer_ID(0) {
        m_verts.reserve(initial_size);
    }

    virtual ~VertList_Base() { reset(); }

    void addQuad(const T(&items)[4]);
    void add(const T *items, int count);
    void reset();
    bool realize();
    bool unrealize();

    bool isRealized()    const { return (m_vertex_buffer_ID != 0); }
    int  getItemCount()  const { return m_verts.size(); }
    int  getByteCount()  const { return m_verts.size() * sizeof(T); }
    int  getTriCount()   const { return m_verts.size() / 3; }
    int  getBradyCount() const { return m_verts.size() / 24; }
    GLuint getVertexBufferID() const { return m_vertex_buffer_ID; }

private:
    // Disallow default ctor, moving and copying.
    VertList_Base() = delete;
    VertList_Base(const VertList_Base &that)  = delete;
    void operator=(const VertList_Base &that) = delete;
    VertList_Base(VertList_Base &&that)  = delete;
    void operator=(VertList_Base &&that) = delete;

    // Private data.
    GLuint m_vertex_buffer_ID;
    std::vector<T> m_verts;
};


// Add a quad. Just a thin wrapper for the skybox.
// TODO: You might want to kill this off later.
template<typename T>
void VertList_Base<T>::addQuad(const T(&items)[4])
{
    std::array<T, 6> stuff = {
        items[0], items[1], items[2],
        items[2], items[1], items[3],
    };

    add(&stuff[0], 6);
}


// Add some triangles. We must NOT be realized yet.
template<typename T>
void VertList_Base<T>::add(const T *items, int count)
{
    assert(m_vertex_buffer_ID == 0);
    m_verts.insert(m_verts.end(), items, items + count);
}


// Empty out everything.
// If we are realized, that's okay, but free that up first.
template<typename T>
void VertList_Base<T>::reset()
{
    if (m_vertex_buffer_ID != 0) {
        glDeleteBuffers(1, &m_vertex_buffer_ID);
        m_vertex_buffer_ID = 0;
    }

    m_verts.clear();
}


// Send our data out to the video card.
// We should only do this if we have data, and haven't done it already.
template<typename T>
bool VertList_Base<T>::realize()
{
    if (m_verts.size() == 0) {
        return false;
    }

    if (m_vertex_buffer_ID != 0) {
        return false;
    }

    // Get our buffer ID.
    glGenBuffers(1, &m_vertex_buffer_ID);

    // Update the buffer.
    int byte_count = m_verts.size() * sizeof(T);
    int item_count = m_verts.size();

    // FUCK. Where does this come from?
    if (item_count > 1000000) {
        printf("%d", item_count);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_ID);
    glBufferData(GL_ARRAY_BUFFER, byte_count, &m_verts[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}


// Clear our data out of the video card, but leave our vertices intact.
template<typename T>
bool VertList_Base<T>::unrealize()
{
    if (m_vertex_buffer_ID == 0) {
        return false;
    }

    glDeleteBuffers(1, &m_vertex_buffer_ID);
    m_vertex_buffer_ID = 0;
    return true;
}

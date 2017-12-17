#pragma once

#include "stdafx.h"


// A call to "clear" can reset these, but otherwise, we'll alway rebuild from scratch.
// Since this is a template, *all* of the code has to be in a header file.
template<typename T>
class VertList_Base
{
public:
    VertList_Base(int initial_size) :
        m_vertex_buffer_ID(0),
        m_item_count(0),
        m_triangle_count(0) {
        m_verts.reserve(initial_size);
    }

    virtual ~VertList_Base() { reset(); }

    void addQuad(const T(&items)[4]);
    void add(const T *items, int item_count, int triangle_count);
    void reset();
    bool realize();
    bool unrealize();

    bool hasData()    const { return (m_verts.size() > 0); }
    bool isRealized() const { return (m_vertex_buffer_ID != 0); }

    int    getByteCount()      const { return m_item_count * sizeof(T); }
    int    getItemCount()      const { return m_item_count; }
    int    getTriangleCount()  const { return m_triangle_count; }
    GLuint getVertexBufferID() const { return m_vertex_buffer_ID; }

private:
    // Disallow default ctor, moving and copying.
    VertList_Base() = delete;
    VertList_Base(const VertList_Base &that)  = delete;
    void operator=(const VertList_Base &that) = delete;
    VertList_Base(VertList_Base &&that)  = delete;
    void operator=(VertList_Base &&that) = delete;

    GLuint m_vertex_buffer_ID;
    int    m_item_count;
    int    m_triangle_count;

    std::vector<T> m_verts;
};


// Add a quad.
// We must NOT be realized yet. You should check first.
template<typename T>
void VertList_Base<T>::addQuad(const T(&items)[4])
{
    assert(NOT(isRealized()));

    T quad_array[6] = {
        items[0], items[1], items[2],
        items[2], items[1], items[3],
    };

    m_verts.insert(m_verts.end(), &quad_array[0], &quad_array[6]);
    m_item_count     += 6;
    m_triangle_count += 3;
}


// Add an arbitrary list of items. Call this when creating triangle strips.
// We must NOT be realized yet. You should check first.
template<typename T>
void VertList_Base<T>::add(const T *items, int item_count, int triangle_count)
{
    assert(NOT(isRealized()));

    m_verts.insert(m_verts.end(), &items[0], &items[item_count]);
    m_item_count     += item_count;
    m_triangle_count += triangle_count;
}


// Empty out everything.
template<typename T>
void VertList_Base<T>::reset()
{
    // If we are realized, that's okay. Free that up.
    if (isRealized()) {
        glDeleteBuffers(1, &m_vertex_buffer_ID);
        m_vertex_buffer_ID = 0;
    }

    m_verts.clear();
    m_item_count     = 0;
    m_triangle_count = 0;
}


// Send our data out to the video card.
// We should only do this if we have data, and haven't done it already.
template<typename T>
bool VertList_Base<T>::realize()
{
    if (NOT(hasData())) {
        return false;
    }
    if (isRealized()) {
        return false;
    }

    // Get our buffer ID.
    glGenBuffers(1, &m_vertex_buffer_ID);

    // Update the buffer.
    int byte_count = m_verts.size() * sizeof(T);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_ID);
    glBufferData(GL_ARRAY_BUFFER, byte_count, &m_verts[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}


// Clear our data out of the video card, but leave our vertices intact.
template<typename T>
bool VertList_Base<T>::unrealize()
{
    if (NOT(isRealized())) {
        return false;
    }

    glDeleteBuffers(1, &m_vertex_buffer_ID);
    m_vertex_buffer_ID = 0;
    return true;
}

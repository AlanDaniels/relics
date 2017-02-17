#pragma once

#include "stdafx.h"


// A call to "clear" can reset these, but otherwise, we'll alway rebuild from scratch.
// Since this is a template, *all* of the code has to be in a header file.
template<class T>
class VertList_Base
{
public:
    VertList_Base() :
        m_vertex_buffer_ID(0), m_item_count(0),
        m_triangle_count(0),   m_up_to_date(true) {}

    virtual ~VertList_Base() { clear(); }

    void addQuad(const T(&items)[4]);
    void add(const T *items, int item_count, int triangle_count);
    void clear();
    void update();

    bool   isUpToDate()        const { return m_up_to_date; }
    int    getByteCount()      const { return m_item_count * sizeof(T); }
    int    getItemCount()      const { return m_item_count; }
    int    getTriangleCount()  const { return m_triangle_count; }
    GLuint getVertexBufferID() const { return m_vertex_buffer_ID; }

private:
    // Disallow copying.
    VertList_Base(const VertList_Base &that)  = delete;
    void operator=(const VertList_Base &that) = delete;

    std::vector<T> m_verts;

    GLuint m_vertex_buffer_ID;
    int    m_item_count;
    int    m_triangle_count;
    bool   m_up_to_date;
};


// Add a quad.
template<class T>
void VertList_Base<T>::addQuad(const T(&items)[4])
{
    assert(m_vertex_buffer_ID == 0);

    // Add the quad.
    T quad_array[6] = {
        items[0], items[1], items[2],
        items[2], items[1], items[3],
    };

    m_verts.insert(m_verts.end(), &quad_array[0], &quad_array[6]);

    m_item_count     += 6;
    m_triangle_count += 3;
    m_up_to_date = false;
}


// Add an arbitrary list of items. Call this when creating triangle strips.
template<class T>
void VertList_Base<T>::add(const T *items, int item_count, int triangle_count)
{
    assert(m_vertex_buffer_ID == 0);

    m_verts.insert(m_verts.end(), &items[0], &items[item_count]);

    m_item_count     += item_count;
    m_triangle_count += triangle_count;
    m_up_to_date = false;
}


// Empty out everything.
template<class T>
void VertList_Base<T>::clear()
{
    // Free up our buffer ID, if any.
    if (m_vertex_buffer_ID != 0) {
        glDeleteBuffers(1, &m_vertex_buffer_ID);
        m_vertex_buffer_ID = 0;
    }

    m_verts.clear();
    m_item_count     = 0;
    m_triangle_count = 0;
    m_up_to_date = true;
}


// Update our buffer data.
template<class T>
void VertList_Base<T>::update()
{
    // If we're already up-to-date, then never mind.
    if (m_up_to_date) {
        return;
    }

    // Get our buffer ID.
    assert(m_vertex_buffer_ID == 0);
    glGenBuffers(1, &m_vertex_buffer_ID);

    // Update the buffer.
    int byte_count = m_verts.size() * sizeof(T);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_ID);
    glBufferData(GL_ARRAY_BUFFER, byte_count, &m_verts[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_up_to_date = true;
}

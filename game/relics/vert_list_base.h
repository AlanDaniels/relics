#pragma once

#include "stdafx.h"


// A call to "clear" can reset these, but otherwise, we'll alway rebuild from scratch.
// Since this is a template, *all* of the code has to be in a header file.
template<typename T>
class VertList_Base
{
public:
    VertList_Base();

    VertList_Base(VertList_Base &&that) :
        m_vertex_buffer_ID(that.m_vertex_buffer_ID),
        m_current(that.m_current),
        m_verts(std::move(that.m_verts)) {}

    virtual ~VertList_Base();

    VertList_Base &operator=(VertList_Base &&that) {
        m_vertex_buffer_ID = that.m_vertex_buffer_ID;
        m_current = that.m_current;
        m_verts   = std::move(that.m_verts);
        return *this;
    }

    void addQuad(const T(&items)[4]);
    void add(const T *items, int count);
    void reset();
    bool update();

    void reserve(int cap) { m_verts.reserve(cap); }

    bool isCurrent()    const { return m_current; }
    int  getItemCount() const { return m_verts.size(); }
    int  getByteCount() const { return m_verts.size() * sizeof(T); }
    int  getTriCount()  const { return m_verts.size() / 3; }

    GLuint getVertexBufferID() const { return m_vertex_buffer_ID; }
    const std::vector<T> &getVerts() const { return m_verts; }

private:
    DISALLOW_COPYING(VertList_Base)

    // Private data.
    GLuint m_vertex_buffer_ID;
    bool   m_current;
    std::vector<T> m_verts;
};


// Default ctor. Get our buffer ID.
template<typename T>
VertList_Base<T>::VertList_Base() :
    m_vertex_buffer_ID(0),
    m_current(false)
{
    glGenBuffers(1, &m_vertex_buffer_ID);
    assert(m_vertex_buffer_ID != 0);
}


// Destructor.
template<typename T>
VertList_Base<T>::~VertList_Base()
{
    glDeleteBuffers(1, &m_vertex_buffer_ID);
    m_vertex_buffer_ID = 0;
}



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


// Add some triangles.
template<typename T>
void VertList_Base<T>::add(const T *items, int count)
{
    m_current = false;
    m_verts.insert(m_verts.end(), items, items + count);
}


// Empty out everything.
// If we are realized, that's okay, but free that up first.
template<typename T>
void VertList_Base<T>::reset()
{
    if (m_verts.size() > 0) {
        m_current = false;
        m_verts.clear();
    }
}


// Send our data out to the video card.
// We should only do this if we have data, and haven't done it already.
template<typename T>
bool VertList_Base<T>::update()
{
    if (m_verts.size() == 0) {
        m_current = true;
        return false;
    }

    if (m_current) {
        return false;
    }

    // Update the buffer.
    int byte_count = m_verts.size() * sizeof(T);
    int item_count = m_verts.size();

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_ID);
    glBufferData(GL_ARRAY_BUFFER, byte_count, &m_verts.at(0), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_current = true;
    return true;
}

#pragma once

#include "stdafx.h"

#include "block.h"
#include "draw_state_pnt.h"

class Chunk;


// Everything dealing with Surface Lists must only be called
// from the main thread, since they involve OpenGL buffers.
class Landscape
{
public:
    Landscape(Chunk &owner);
    ~Landscape();

    int getCountForSurface(SurfaceType surf) const;
    const VertList_PNT *getSurfaceList_RO(SurfaceType surf) const;
    VertList_PNT &getSurfaceList_RW(SurfaceType surf);
    void rebuildSurfaceLists();
    void freeSurfaceLists();

private:
    FORBID_DEFAULT_CTOR(Landscape)
    FORBID_COPYING(Landscape)
    FORBID_MOVING(Landscape)

    // Private data
    Chunk &m_owner;

    std::array<std::unique_ptr<VertList_PNT>, SURFACE_TYPE_COUNT> m_vert_lists;
};
#pragma once

#include "my_math.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"


class Block;
class Chunk;
enum  FaceEnum;
enum  EdgeEnum;


// The 9-by-9 array of quads.
// Doing this in a class to make the calculations more sane.
class Brady
{
public:
    static const int VERTS_PER_BRADY = 24;

    Brady(const Chunk &chunk, const LocalGrid &local_coord, FaceEnum face);
    std::string toString() const;

    void addToVertList_PNT(VertList_PNT *pOut);
    void addToVertList_PT(VertList_PT *pOut);

private:
    // Forbid default ctor, moving and copying.
    Brady() = delete;
    Brady(const Brady &that) = delete;
    void operator=(const Brady &that) = delete;
    Brady(Brady &&that) = delete;
    void operator=(Brady &&that) = delete;

    LocalGrid  m_local_coord;
    FaceEnum   m_face;
    Vertex_PNT m_verts[4];
};
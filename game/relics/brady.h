#pragma once

#include "my_math.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"


class  Chunk;
enum   FaceEnum;
enum   EdgeEnum;


// The 9-by-9 array of quads.
// Doing this in a class to make the calculations more sane.
class Brady
{
public:
    Brady(const Chunk &chunk, const MyGridCoord &local_coord, FaceEnum face);
    std::string toString() const;

    void addToVertList_PNT(VertList_PNT *pOut);
    void addToVertList_PT(VertList_PT *pOut);

private:
    // Forbid copying, and default ctor.
    Brady() = delete;
    Brady(const Brady &that) = delete;
    void operator=(const Brady &that) = delete;

    MyGridCoord m_local_coord;
    FaceEnum m_face;

    Vertex_PNT m_verts[4][4];
};
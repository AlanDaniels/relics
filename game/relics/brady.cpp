
#include "stdafx.h"
#include "brady.h"
#include "common_util.h"

#include "chunk.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"
#include "config.h"
#include "simplex_noise.h"
#include "utils.h"


const GLfloat ONE_THIRD  = 0.3333333f;
const GLfloat TWO_THIRDS = 0.6666667f;


// Constructor from values.
Brady::Brady(const Chunk &chunk, const LocalGrid &local_coord, FaceEnum face) :
    m_local_coord(local_coord),
    m_face(face)
{
    int x = local_coord.x();
    int y = local_coord.y();
    int z = local_coord.z();

    MyVec4 point_ll;
    MyVec4 point_lr;
    MyVec4 point_ul;
    MyVec4 point_ur;

    switch (face) {
    case FACE_TOP:
        point_ll = chunk.localGridToWorldPos(x,     y + 1, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ul = chunk.localGridToWorldPos(x,     y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FACE_BOTTOM:
        point_ll = chunk.localGridToWorldPos(x,     y, z + 1);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z + 1);
        point_ul = chunk.localGridToWorldPos(x,     y, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y, z);
        break;

    case FACE_NORTH:
        point_ll = chunk.localGridToWorldPos(x + 1, y,     z + 1);
        point_lr = chunk.localGridToWorldPos(x,     y,     z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x,     y + 1, z + 1);
        break;

    case FACE_SOUTH:
        point_ll = chunk.localGridToWorldPos(x,     y, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z);
        point_ul = chunk.localGridToWorldPos(x,     y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z);
        break;

    case FACE_EAST:
        point_ll = chunk.localGridToWorldPos(x + 1, y,     z);
        point_lr = chunk.localGridToWorldPos(x + 1, y,     z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FACE_WEST:
        point_ll = chunk.localGridToWorldPos(x, y,     z + 1);
        point_lr = chunk.localGridToWorldPos(x, y,     z);
        point_ul = chunk.localGridToWorldPos(x, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x, y + 1, z);
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        break;
    }

    // Calculate our sixteen points.
    m_verts[0].position = point_ll;
    m_verts[1].position = point_ul;
    m_verts[2].position = point_lr;
    m_verts[3].position = point_ur;

    // UVs.
    m_verts[0].texuv = MyVec2(0.0f, 0.0f);
    m_verts[1].texuv = MyVec2(0.0f, 1.0f); 
    m_verts[2].texuv = MyVec2(1.0f, 0.0f);
    m_verts[3].texuv = MyVec2(1.0f, 1.0f);

    // All our normals will face in the same direction.
    MyVec4 dir_normal;
    switch (face) {
    case FACE_TOP:    dir_normal = VEC4_UPWARD;    break;
    case FACE_BOTTOM: dir_normal = VEC4_DOWNWARD;  break;
    case FACE_NORTH:  dir_normal = VEC4_NORTHWARD; break;
    case FACE_SOUTH:  dir_normal = VEC4_SOUTHWARD; break;
    case FACE_EAST:   dir_normal = VEC4_EASTWARD;  break;
    case FACE_WEST:   dir_normal = VEC4_WESTWARD;  break;
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        dir_normal = VEC4_UPWARD;
        break;
    }

    for (int i = 0; i < 4; i++) {
        m_verts[i].normal = dir_normal;
    }
}


// Print this out, for debugging.
std::string Brady::toString() const
{
    std::string result;
    std::unique_ptr<char[]> buffer(new char[1024]);

    LocalGrid local_coord = m_local_coord;
    sprintf(buffer.get(),
        "Patch for [%d %d %d], face %s\n",
        local_coord.x(), local_coord.y(), local_coord.z(),
        FaceEnumToString(m_face));
    result += buffer.get();

    result += "Points:\n";

    for (int i = 0; i < 4; i++) {
        sprintf(buffer.get(), "  %s\n", m_verts[i].position.toString().c_str());
        result += buffer.get();
    }

    return result;
}


// Back to boring patches.
void Brady::addToVertList_PNT(VertList_PNT *pOut) 
{
    std::array<Vertex_PNT, 6> verts = {
        m_verts[0], m_verts[3], m_verts[1],
        m_verts[3], m_verts[0], m_verts[2]
    };

    pOut->add(&verts[0], 6);
}


// Copied logic from the "PNT" version, but "down-casted" the vertex type.
void Brady::addToVertList_PT(VertList_PT *pOut)
{
    // Copy over the contents the hard way, I suppose.
    Vertex_PT temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i].position = m_verts[i].position;
        temp[i].texuv    = m_verts[i].texuv;
    }

    std::array<Vertex_PT, 6> verts = {
        temp[0], temp[3], temp[1],
        temp[3], temp[0], temp[2]
    };

    pOut->add(&verts[0], 6);
}

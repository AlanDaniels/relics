
#include "stdafx.h"
#include "add_face.h"
#include "common_util.h"

#include "chunk.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"
#include "config.h"
#include "simplex_noise.h"
#include "utils.h"


void AddFace_VertList_PNT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceEnum face, 
    VertList_PNT *pOut)
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

    // Position, normal, texuv.
    std::array<Vertex_PNT, 6> triangles = {
        Vertex_PNT(point_ll, dir_normal, MyVec2(0.0f, 0.0f)), // LL
        Vertex_PNT(point_ur, dir_normal, MyVec2(1.0f, 1.0f)), // UR
        Vertex_PNT(point_ul, dir_normal, MyVec2(0.0f, 1.0f)), // UL
        Vertex_PNT(point_ur, dir_normal, MyVec2(1.0f, 1.0f)), // UR
        Vertex_PNT(point_ll, dir_normal, MyVec2(0.0f, 0.0f)), // LL
        Vertex_PNT(point_lr, dir_normal, MyVec2(1.0f, 0.0f)), // LR
    };


    pOut->add(&triangles[0], 6);
}


void AddFace_VertList_PT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceEnum face,
    VertList_PT *pOut)
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
        point_ll = chunk.localGridToWorldPos(x, y + 1, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ul = chunk.localGridToWorldPos(x, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FACE_BOTTOM:
        point_ll = chunk.localGridToWorldPos(x, y, z + 1);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z + 1);
        point_ul = chunk.localGridToWorldPos(x, y, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y, z);
        break;

    case FACE_NORTH:
        point_ll = chunk.localGridToWorldPos(x + 1, y, z + 1);
        point_lr = chunk.localGridToWorldPos(x, y, z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x, y + 1, z + 1);
        break;

    case FACE_SOUTH:
        point_ll = chunk.localGridToWorldPos(x, y, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z);
        point_ul = chunk.localGridToWorldPos(x, y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z);
        break;

    case FACE_EAST:
        point_ll = chunk.localGridToWorldPos(x + 1, y, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FACE_WEST:
        point_ll = chunk.localGridToWorldPos(x, y, z + 1);
        point_lr = chunk.localGridToWorldPos(x, y, z);
        point_ul = chunk.localGridToWorldPos(x, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x, y + 1, z);
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        break;
    }

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

    // Position, normal, texuv.
    std::array<Vertex_PT, 6> triangles = {
        Vertex_PT(point_ll, MyVec2(0.0f, 0.0f)), // LL
        Vertex_PT(point_ur, MyVec2(1.0f, 1.0f)), // UR
        Vertex_PT(point_ul, MyVec2(0.0f, 1.0f)), // UL
        Vertex_PT(point_ur, MyVec2(1.0f, 1.0f)), // UR
        Vertex_PT(point_ll, MyVec2(0.0f, 0.0f)), // LL
        Vertex_PT(point_lr, MyVec2(1.0f, 0.0f)), // LR
    };

    pOut->add(&triangles[0], 6);
}


#include "stdafx.h"
#include "add_face.h"

#include "common_util.h"
#include "chunk.h"
#include "utils.h"


std::array<Vertex_PNT, 6> GetLandscapePatch_PNT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceType face)
{
    int x = local_coord.x();
    int y = local_coord.y();
    int z = local_coord.z();

    MyVec4 point_ll;
    MyVec4 point_lr;
    MyVec4 point_ul;
    MyVec4 point_ur;

    switch (face) {
    case FaceType::TOP:
        point_ll = chunk.localGridToWorldPos(x,     y + 1, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ul = chunk.localGridToWorldPos(x,     y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FaceType::BOTTOM:
        point_ll = chunk.localGridToWorldPos(x,     y, z + 1);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z + 1);
        point_ul = chunk.localGridToWorldPos(x,     y, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y, z);
        break;

    case FaceType::NORTH:
        point_ll = chunk.localGridToWorldPos(x + 1, y,     z + 1);
        point_lr = chunk.localGridToWorldPos(x,     y,     z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x,     y + 1, z + 1);
        break;

    case FaceType::SOUTH:
        point_ll = chunk.localGridToWorldPos(x,     y, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z);
        point_ul = chunk.localGridToWorldPos(x,     y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z);
        break;

    case FaceType::EAST:
        point_ll = chunk.localGridToWorldPos(x + 1, y,     z);
        point_lr = chunk.localGridToWorldPos(x + 1, y,     z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FaceType::WEST:
        point_ll = chunk.localGridToWorldPos(x, y,     z + 1);
        point_lr = chunk.localGridToWorldPos(x, y,     z);
        point_ul = chunk.localGridToWorldPos(x, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x, y + 1, z);
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(face));
        break;
    }

    // All our normals will face in the same direction.
    MyVec4 dir_normal;
    switch (face) {
    case FaceType::TOP:    dir_normal = VEC4_UPWARD;    break;
    case FaceType::BOTTOM: dir_normal = VEC4_DOWNWARD;  break;
    case FaceType::NORTH:  dir_normal = VEC4_NORTHWARD; break;
    case FaceType::SOUTH:  dir_normal = VEC4_SOUTHWARD; break;
    case FaceType::EAST:   dir_normal = VEC4_EASTWARD;  break;
    case FaceType::WEST:   dir_normal = VEC4_WESTWARD;  break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(face));
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


    return std::move(triangles);
}


std::array<Vertex_PT, 6> GetLandscapePatch_PT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceType face)
{
    int x = local_coord.x();
    int y = local_coord.y();
    int z = local_coord.z();

    MyVec4 point_ll;
    MyVec4 point_lr;
    MyVec4 point_ul;
    MyVec4 point_ur;

    switch (face) {
    case FaceType::TOP:
        point_ll = chunk.localGridToWorldPos(x,     y + 1, z);
        point_lr = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ul = chunk.localGridToWorldPos(x,     y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FaceType::BOTTOM:
        point_ll = chunk.localGridToWorldPos(x,     y, z + 1);
        point_lr = chunk.localGridToWorldPos(x + 1, y, z + 1);
        point_ul = chunk.localGridToWorldPos(x,     y, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y, z);
        break;

    case FaceType::NORTH:
        point_ll = chunk.localGridToWorldPos(x + 1, y,     z + 1);
        point_lr = chunk.localGridToWorldPos(x,     y,     z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x,     y + 1, z + 1);
        break;

    case FaceType::SOUTH:
        point_ll = chunk.localGridToWorldPos(x,     y,     z);
        point_lr = chunk.localGridToWorldPos(x + 1, y,     z);
        point_ul = chunk.localGridToWorldPos(x,     y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z);
        break;

    case FaceType::EAST:
        point_ll = chunk.localGridToWorldPos(x + 1, y,     z);
        point_lr = chunk.localGridToWorldPos(x + 1, y,     z + 1);
        point_ul = chunk.localGridToWorldPos(x + 1, y + 1, z);
        point_ur = chunk.localGridToWorldPos(x + 1, y + 1, z + 1);
        break;

    case FaceType::WEST:
        point_ll = chunk.localGridToWorldPos(x, y,     z + 1);
        point_lr = chunk.localGridToWorldPos(x, y,     z);
        point_ul = chunk.localGridToWorldPos(x, y + 1, z + 1);
        point_ur = chunk.localGridToWorldPos(x, y + 1, z);
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(face));
        break;
    }

    // All our normals will face in the same direction.
    MyVec4 dir_normal;
    switch (face) {
    case FaceType::TOP:    dir_normal = VEC4_UPWARD;    break;
    case FaceType::BOTTOM: dir_normal = VEC4_DOWNWARD;  break;
    case FaceType::NORTH:  dir_normal = VEC4_NORTHWARD; break;
    case FaceType::SOUTH:  dir_normal = VEC4_SOUTHWARD; break;
    case FaceType::EAST:   dir_normal = VEC4_EASTWARD;  break;
    case FaceType::WEST:   dir_normal = VEC4_WESTWARD;  break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(face));
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

    return std::move(triangles);
}

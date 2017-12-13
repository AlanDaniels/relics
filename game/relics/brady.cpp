
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
        point_ll = chunk.localToWorldCoord(x,     y + 1, z);
        point_lr = chunk.localToWorldCoord(x + 1, y + 1, z);
        point_ul = chunk.localToWorldCoord(x,     y + 1, z + 1);
        point_ur = chunk.localToWorldCoord(x + 1, y + 1, z + 1);
        break;

    case FACE_BOTTOM:
        point_ll = chunk.localToWorldCoord(x,     y, z + 1);
        point_lr = chunk.localToWorldCoord(x + 1, y, z + 1);
        point_ul = chunk.localToWorldCoord(x,     y, z);
        point_ur = chunk.localToWorldCoord(x + 1, y, z);
        break;

    case FACE_NORTH:
        point_ll = chunk.localToWorldCoord(x + 1, y,     z + 1);
        point_lr = chunk.localToWorldCoord(x,     y,     z + 1);
        point_ul = chunk.localToWorldCoord(x + 1, y + 1, z + 1);
        point_ur = chunk.localToWorldCoord(x,     y + 1, z + 1);
        break;

    case FACE_SOUTH:
        point_ll = chunk.localToWorldCoord(x,     y, z);
        point_lr = chunk.localToWorldCoord(x + 1, y, z);
        point_ul = chunk.localToWorldCoord(x,     y + 1, z);
        point_ur = chunk.localToWorldCoord(x + 1, y + 1, z);
        break;

    case FACE_EAST:
        point_ll = chunk.localToWorldCoord(x + 1, y,     z);
        point_lr = chunk.localToWorldCoord(x + 1, y,     z + 1);
        point_ul = chunk.localToWorldCoord(x + 1, y + 1, z);
        point_ur = chunk.localToWorldCoord(x + 1, y + 1, z + 1);
        break;

    case FACE_WEST:
        point_ll = chunk.localToWorldCoord(x, y,     z + 1);
        point_lr = chunk.localToWorldCoord(x, y,     z);
        point_ul = chunk.localToWorldCoord(x, y + 1, z + 1);
        point_ur = chunk.localToWorldCoord(x, y + 1, z);
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        break;
    }

    // Calculate our sixteen points.
    m_verts[0][0].position = point_ll;
    m_verts[1][0].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, ONE_THIRD,  0.0f);
    m_verts[2][0].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, TWO_THIRDS, 0.0f);
    m_verts[3][0].position = point_lr;

    m_verts[0][1].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, 0.0f,       ONE_THIRD);
    m_verts[1][1].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, ONE_THIRD,  ONE_THIRD);
    m_verts[2][1].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, TWO_THIRDS, ONE_THIRD);
    m_verts[3][1].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, 1.0f,       ONE_THIRD);

    m_verts[0][2].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, 0.0f,       TWO_THIRDS);
    m_verts[1][2].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, ONE_THIRD,  TWO_THIRDS);
    m_verts[2][2].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, TWO_THIRDS, TWO_THIRDS);
    m_verts[3][2].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, 1.0f,       TWO_THIRDS);

    m_verts[0][3].position = point_ul;
    m_verts[1][3].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, ONE_THIRD,  1.0f);
    m_verts[2][3].position = FourWayLerp4(point_ll, point_lr, point_ul, point_ur, TWO_THIRDS, 1.0f);
    m_verts[3][3].position = point_ur;

    // Add some landscape noise to displace the positions.
    GLfloat how_much = GetConfig().logic.getLandscapeNoiseCm();
    if (how_much > 0.0f) {
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                // Scale out to the block size, so our noise is reasonably sized.
                // The "1.1" and such is so there's a little variety in the three directions.
                MyVec4 sample_x = m_verts[x][y].position.dividedBy(BLOCK_SCALE);
                MyVec4 sample_y = m_verts[x][y].position.dividedBy(BLOCK_SCALE * 1.1f);
                MyVec4 sample_z = m_verts[x][y].position.dividedBy(BLOCK_SCALE * 1.2f);

                GLfloat noise_x = simplex_noise_3(sample_x);
                GLfloat noise_y = simplex_noise_3(sample_y);
                GLfloat noise_z = simplex_noise_3(sample_z);

                MyVec4 new_pos(
                    m_verts[x][y].position.x() + (how_much * (1.0f - (0.5f * noise_x))),
                    m_verts[x][y].position.y() + (how_much * (1.0f - (0.5f * noise_y))),
                    m_verts[x][y].position.z() + (how_much * (1.0f - (0.5f * noise_z))),
                    1.0f);

                m_verts[x][y].position = new_pos;
            }
        }
    }

    // TODO: Continue here. 
    // Figure out how to nudge the edges, but only after getting everything else working flawlessly.

    // Calculate our sixteen UVs.
    MyVec2 uv_ll(0.0f, 0.0f);
    MyVec2 uv_lr(1.0f, 0.0f);
    MyVec2 uv_ul(0.0f, 1.0f);
    MyVec2 uv_ur(1.0f, 1.0f);

    m_verts[0][0].texuv = uv_ll;
    m_verts[1][0].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, ONE_THIRD,  0.0f);
    m_verts[2][0].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, TWO_THIRDS, 0.0f);
    m_verts[3][0].texuv = uv_lr;

    m_verts[0][1].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, 0.0f,       ONE_THIRD);
    m_verts[1][1].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, ONE_THIRD,  ONE_THIRD);
    m_verts[2][1].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, TWO_THIRDS, ONE_THIRD);
    m_verts[3][1].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, 1.0f,       ONE_THIRD);

    m_verts[0][2].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, 0.0f,       TWO_THIRDS);
    m_verts[1][2].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, ONE_THIRD,  TWO_THIRDS);
    m_verts[2][2].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, TWO_THIRDS, TWO_THIRDS);
    m_verts[3][2].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, 1.0f,       TWO_THIRDS);

    m_verts[0][3].texuv = uv_ul;
    m_verts[1][3].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, ONE_THIRD,  1.0f);
    m_verts[2][3].texuv = FourWayLerp2(uv_ll, uv_lr, uv_ul, uv_ur, TWO_THIRDS, 1.0f);
    m_verts[3][3].texuv = uv_ur;

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

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            m_verts[x][y].normal = dir_normal;
        }
    }
}


// Print this out, for debugging.
std::string Brady::toString() const
{
    std::string result;
    char buffer[128];

    sprintf(buffer,
        "Brady for [%d %d %d], face %s\n",
        m_local_coord.x(), m_local_coord.y(), m_local_coord.z(),
        FaceEnumToString(m_face));
    result += buffer;

    sprintf(buffer, "Points:\n");
    result += buffer;

    for (int y = 3; y >= 0; y--) {
        sprintf(buffer, "  [%s] [%s] [%s] [%s]\n",
            m_verts[0][y].position.toString().c_str(),
            m_verts[1][y].position.toString().c_str(),
            m_verts[2][y].position.toString().c_str(),
            m_verts[3][y].position.toString().c_str());
        result += buffer;
    }

    sprintf(buffer, "UVs:\n");
    result += buffer;

    for (int y = 3; y >= 0; y--) {
        sprintf(buffer, "  [%s] [%s] [%s] [%s]\n",
            m_verts[0][y].texuv.toString().c_str(),
            m_verts[1][y].texuv.toString().c_str(),
            m_verts[2][y].texuv.toString().c_str(),
            m_verts[3][y].texuv.toString().c_str());
        result += buffer;
    }

    return result;
}


// Note that we're now using triangle strips.
void Brady::addToVertList_PNT(VertList_PNT *pOut) 
{
    // If you ever get lost, draw this out on paper again.
    // Repeat twice at the beginning, and end, to seal off the strips.
    Vertex_PNT verts[] = {
        m_verts[0][0],

        m_verts[0][0],
        m_verts[0][1],
        m_verts[1][0],
        m_verts[1][1],
        m_verts[2][0],
        m_verts[2][1],
        m_verts[3][0],
        m_verts[3][1],

        m_verts[3][2],
        m_verts[2][1],
        m_verts[2][2],
        m_verts[1][1],
        m_verts[1][2],
        m_verts[0][1],
        m_verts[0][2],

        m_verts[0][3],
        m_verts[1][2],
        m_verts[1][3],
        m_verts[2][2],
        m_verts[2][3],
        m_verts[3][2],
        m_verts[3][3],

        m_verts[3][3]
    };

    int vert_count = 24;
    int triangle_count = 18;
    pOut->add(verts, vert_count, triangle_count);
}


// Copied logic from the "PNT" version, but "down-casted" the vertex type.
void Brady::addToVertList_PT(VertList_PT *pOut)
{
    // Copy over the contents the hard way, I suppose.
    Vertex_PT temp[4][4];
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            temp[x][y].position = m_verts[x][y].position;
            temp[x][y].texuv    = m_verts[x][y].texuv;
        }
    }

    Vertex_PT verts[] = {
        temp[0][0],

        temp[0][0],
        temp[0][1],
        temp[1][0],
        temp[1][1],
        temp[2][0],
        temp[2][1],
        temp[3][0],
        temp[3][1],

        temp[3][2],
        temp[2][1],
        temp[2][2],
        temp[1][1],
        temp[1][2],
        temp[0][1],
        temp[0][2],

        temp[0][3],
        temp[1][2],
        temp[1][3],
        temp[2][2],
        temp[2][3],
        temp[3][2],
        temp[3][3],

        temp[3][3]
    };

    int vert_count = 24;
    int triangle_count = 18;
    pOut->add(verts, vert_count, triangle_count);
}

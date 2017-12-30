
#include "stdafx.h"
#include "chunk_hit_test.h"
#include "common_util.h"

#include "add_face.h"
#include "game_world.h"
#include "utils.h"


// Chunk Hit Test Details assignment operator.
const ChunkHitTestDetail &ChunkHitTestDetail::operator=(const ChunkHitTestDetail &that)
{
    m_chunk_origin = that.m_chunk_origin;
    m_global_coord = that.m_global_coord;
    m_face   = that.m_face;
    m_impact = that.m_impact;
    m_dist   = that.m_dist;
    return *this;
}


// Hit-test details to a description. Handy for debugging.
std::string ChunkHitTestDetail::toDescription() const
{
    const char *face_to_str;
    switch (m_face) {
    case FaceType::NONE:   face_to_str = "None";   break;
    case FaceType::SOUTH:  face_to_str = "South";  break;
    case FaceType::NORTH:  face_to_str = "North";  break;
    case FaceType::WEST:   face_to_str = "West";   break;
    case FaceType::EAST:   face_to_str = "East";   break;
    case FaceType::TOP:    face_to_str = "Top";    break;
    case FaceType::BOTTOM: face_to_str = "Bottom"; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(m_face));
        return "";
    }

    char msg[128];
    sprintf(msg,
        "Face = %s, block = [%d, %d, %d], impact = [%.0f, %.0f, %.0f], dist = %.0f",
        face_to_str, m_global_coord.x(), m_global_coord.y(), m_global_coord.z(),
        m_impact.x(), m_impact.y(), m_impact.z(), m_dist);

    return std::string(msg);
}


// All of thes hit-testers will return "bool", but deal with
// "HitTestEnum" internally, so that we can still debug what's going on.
// For each, if we hit a block, and that block is not empty, then there's your answer.
// Note that we have to burrow "inward" a bit to find the proper grid coord.


// Try each Z-plane, from near to far.
static bool CheckSouthFaces(
    const Chunk &chunk, const MyRay &eye_ray,
    GlobalGrid *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_z = chunk.getOrigin().z();
    int far_z  = chunk.getOrigin().z() + CHUNK_WIDTH - 1;

    for (int grid_z = near_z; grid_z <= far_z; grid_z++) {
        MyPlane plane = GetSouthGridPlane(grid_z);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            GlobalGrid global_coord = WorldPosToGlobalGrid(impact, NUDGE_NORTH);
            if (chunk.IsGlobalGridWithin(global_coord)) {
                LocalGrid local_coord = GlobalGridToLocal(global_coord, chunk.getOrigin());
                BlockType block_type  = chunk.getBlockType(local_coord);

                if (IsBlockTypeFilled(block_type)) {
                    *pOut_coord    = global_coord;
                    *pOut_impact   = impact;
                    *pOut_distance = distance;
                    return true;
                }
            }
        }
    }

    return false;
}


// Try each Z-plane, from near to far.
static bool CheckNorthFaces(
    const Chunk &chunk, const MyRay &eye_ray,
    GlobalGrid *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_z = chunk.getOrigin().z() + CHUNK_WIDTH;
    int far_z  = chunk.getOrigin().z() + 1;

    for (int grid_z = near_z; grid_z >= far_z; grid_z--) {
        MyPlane plane = GetNorthGridPlane(grid_z);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            GlobalGrid global_coord = WorldPosToGlobalGrid(impact, NUDGE_SOUTH);
            if (chunk.IsGlobalGridWithin(global_coord)) {
                LocalGrid local_coord = GlobalGridToLocal(global_coord, chunk.getOrigin());
                BlockType block_type  = chunk.getBlockType(local_coord);

                if (IsBlockTypeFilled(block_type)) {
                    *pOut_coord    = global_coord;
                    *pOut_impact   = impact;
                    *pOut_distance = distance;
                    return true;
                }
            }
        }
    }

    return false;
}


// Try each X-plane, from nearest to farthest.
static bool CheckWestFaces(
    const Chunk &chunk, const MyRay &eye_ray,
    GlobalGrid *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_x = chunk.getOrigin().x();
    int far_x  = chunk.getOrigin().x() + CHUNK_WIDTH - 1;

    for (int grid_x = near_x; grid_x <= far_x; grid_x++) {
        MyPlane plane = GetWestGridPlane(grid_x);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            GlobalGrid global_coord = WorldPosToGlobalGrid(impact, NUDGE_EAST);
            if (chunk.IsGlobalGridWithin(global_coord)) {
                LocalGrid local_coord = GlobalGridToLocal(global_coord, chunk.getOrigin());
                BlockType block_type  = chunk.getBlockType(local_coord);

                if (IsBlockTypeFilled(block_type)) {
                    *pOut_coord    = global_coord;
                    *pOut_impact   = impact;
                    *pOut_distance = distance;
                    return true;
                }
            }
        }
    }

    return false;
}


// Try each X-plane, from nearest to farthest.
static bool CheckEastFaces(
    const Chunk &chunk, const MyRay &eye_ray,
    GlobalGrid *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_x = chunk.getOrigin().x() + CHUNK_WIDTH;
    int far_x  = chunk.getOrigin().x() + 1;

    for (int grid_x = near_x; grid_x >= far_x; grid_x--) {
        MyPlane plane = GetEastGridPlane(grid_x);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            GlobalGrid global_coord = WorldPosToGlobalGrid(impact, NUDGE_WEST);
            if (chunk.IsGlobalGridWithin(global_coord)) {
                LocalGrid local_coord = GlobalGridToLocal(global_coord, chunk.getOrigin());
                BlockType block_type  = chunk.getBlockType(local_coord);

                if (IsBlockTypeFilled(block_type)) {
                    *pOut_coord    = global_coord;
                    *pOut_impact   = impact;
                    *pOut_distance = distance;
                    return true;
                }
            }
        }
    }

    return false;
}


// Try each Y-plane, from nearest to farthest.
static bool CheckTopFaces(
    const Chunk &chunk, const MyRay &eye_ray,
    GlobalGrid *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int high_y = CHUNK_HEIGHT;
    int low_y  = 1;

    for (int grid_y = high_y; grid_y >= low_y; grid_y--) {
        MyPlane plane = GetTopGridPlane(grid_y);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            GlobalGrid global_coord = WorldPosToGlobalGrid(impact, NUDGE_DOWN);
            if (chunk.IsGlobalGridWithin(global_coord)) {
                LocalGrid local_coord = GlobalGridToLocal(global_coord, chunk.getOrigin());
                BlockType block_type  = chunk.getBlockType(local_coord);

                if (IsBlockTypeFilled(block_type)) {
                    *pOut_coord    = global_coord;
                    *pOut_impact   = impact;
                    *pOut_distance = distance;
                    return true;
                }
            }
        }
    }

    return false;
}


// Try each Y-plane, from lowest to highest.
static bool CheckBottomFaces(
    const Chunk &chunk, const MyRay &eye_ray,
    GlobalGrid *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int low_y  = 0;
    int high_y = CHUNK_HEIGHT - 1;

    for (int grid_y = low_y; grid_y <= high_y; grid_y++) {
        MyPlane plane = GetBottomGridPlane(grid_y);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            GlobalGrid global_coord = WorldPosToGlobalGrid(impact, NUDGE_UP);
            if (chunk.IsGlobalGridWithin(global_coord)) {
                LocalGrid local_coord = GlobalGridToLocal(global_coord, chunk.getOrigin());
                BlockType block_type  = chunk.getBlockType(local_coord);

                if (IsBlockTypeFilled(block_type)) {
                    *pOut_coord    = global_coord;
                    *pOut_impact   = impact;
                    *pOut_distance = distance;
                    return true;
                }
            }
        }
    }

    return false;
}


// The first call available outside this file.
bool DoChunkHitTest(const Chunk &chunk, const MyRay &eye_ray, ChunkHitTestDetail *pOut)
{
    // Test each of the six faces. The winner is whichever is closest.
    FaceType winner  = FaceType::NONE;
    GLfloat  closest = FLT_MAX;

    // South faces.
    GlobalGrid south_coord;
    MyVec4  south_impact;
    GLfloat south_dist = 0.0f;
    bool south_hit = CheckSouthFaces(chunk, eye_ray, &south_coord, &south_impact, &south_dist);
    if (south_hit && (south_dist < closest)) {
        winner  = FaceType::SOUTH;
        closest = south_dist;
    }

    // North faces.
    GlobalGrid north_coord;
    MyVec4  north_impact;
    GLfloat north_dist = 0.0f;
    bool north_hit = CheckNorthFaces(chunk, eye_ray, &north_coord, &north_impact, &north_dist);
    if (north_hit && (north_dist < closest)) {
        winner  = FaceType::NORTH;
        closest = north_dist;
    }

    // West faces.
    GlobalGrid west_coord;
    MyVec4  west_impact;
    GLfloat west_dist = 0.0;
    bool west_hit = CheckWestFaces(chunk, eye_ray, &west_coord, &west_impact, &west_dist);
    if (west_hit && (west_dist < closest)) {
        winner  = FaceType::WEST;
        closest = west_dist;
    }

    // East faces.
    GlobalGrid east_coord;
    MyVec4  east_impact;
    GLfloat east_dist = 0.0;
    bool east_hit = CheckEastFaces(chunk, eye_ray, &east_coord, &east_impact, &east_dist);
    if (east_hit && (east_dist < closest)) {
        winner  = FaceType::EAST;
        closest = east_dist;
    }

    // Top faces.
    GlobalGrid top_coord;
    MyVec4  top_impact;
    GLfloat top_dist   = 0.0;
    bool top_hit = CheckTopFaces(chunk, eye_ray, &top_coord, &top_impact, &top_dist);
    if (top_hit && (top_dist < closest)) {
        winner  = FaceType::TOP;
        closest = top_dist;
    }

    // Bottom faces.
    GlobalGrid bottom_coord;
    MyVec4  bottom_impact;
    GLfloat bottom_dist = 0.0;
    bool bottom_hit = CheckBottomFaces(chunk, eye_ray, &bottom_coord, &bottom_impact, &bottom_dist);
    if (bottom_hit && (bottom_dist < closest)) {
        winner  = FaceType::BOTTOM;
        closest = bottom_dist;
    }

    // All done. Draw the winner.
    const ChunkOrigin &origin = chunk.getOrigin();

    switch (winner) {
    case FaceType::NONE:
        // If we got to here, we didn't hit anything.
        return false;

    case FaceType::SOUTH:
        *pOut = ChunkHitTestDetail(origin, south_coord, FaceType::SOUTH, south_impact, south_dist);
        return true;

    case FaceType::NORTH:
        *pOut = ChunkHitTestDetail(origin, north_coord, FaceType::NORTH, north_impact, north_dist);
        return true;

    case FaceType::WEST:
        *pOut = ChunkHitTestDetail(origin, west_coord, FaceType::WEST, west_impact, west_dist);
        return true;

    case FaceType::EAST:
        *pOut = ChunkHitTestDetail(origin, east_coord, FaceType::EAST, east_impact, east_dist);
        return true;

    case FaceType::TOP:
        *pOut = ChunkHitTestDetail(origin, top_coord, FaceType::TOP, top_impact, top_dist);
        return true;

    case FaceType::BOTTOM:
        *pOut = ChunkHitTestDetail(origin, bottom_coord, FaceType::BOTTOM, bottom_impact, bottom_dist);
        return true;

    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(winner));
        return false;
    }
}


// The second call available outside this file.
void ChunkHitTestToQuad(const Chunk &chunk, const ChunkHitTestDetail &details, VertList_PT *pOut)
{
    GlobalGrid global_coord = details.getGlobalCoord();
    LocalGrid  local_coord  = GlobalGridToLocal(global_coord, chunk.getOrigin());

    pOut->reset();
    auto tris = GetLandscapePatch_PT(chunk, local_coord, details.getFace());
    pOut->add(tris.data(), tris.size());
    pOut->update();
}


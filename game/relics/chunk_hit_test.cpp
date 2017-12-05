
#include "stdafx.h"
#include "chunk_hit_test.h"

#include "brady.h"
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
    case FACE_NONE:   face_to_str = "None";   break;
    case FACE_SOUTH:  face_to_str = "South";  break;
    case FACE_NORTH:  face_to_str = "North";  break;
    case FACE_WEST:   face_to_str = "West";   break;
    case FACE_EAST:   face_to_str = "East";   break;
    case FACE_TOP:    face_to_str = "Top";    break;
    case FACE_BOTTOM: face_to_str = "Bottom"; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, m_face);
        return std::string("");
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
    MyGridCoord *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_z = chunk.getOrigin().z();
    int far_z  = chunk.getOrigin().z() + CHUNK_DEPTH_Z - 1;

    for (int grid_z = near_z; grid_z <= far_z; grid_z++) {
        MyPlane plane = GetSouthGridPlane(grid_z);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            MyGridCoord coord = WorldToGridCoord(impact, NUDGE_NORTH);
            if (chunk.isCoordWithin(coord)) {
                const Block *block = chunk.getBlockGlobal_RO(coord);
                if (block->isFilled()) {
                    *pOut_coord    = coord;
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
    MyGridCoord *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_z = chunk.getOrigin().z() + CHUNK_DEPTH_Z;
    int far_z  = chunk.getOrigin().z() + 1;

    for (int grid_z = near_z; grid_z >= far_z; grid_z--) {
        MyPlane plane = GetNorthGridPlane(grid_z);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            MyGridCoord coord = WorldToGridCoord(impact, NUDGE_SOUTH);
            if (chunk.isCoordWithin(coord)) {
                const Block *block = chunk.getBlockGlobal_RO(coord);
                if (block->isFilled()) {
                    *pOut_coord    = coord;
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
    MyGridCoord *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_x = chunk.getOrigin().x();
    int far_x  = chunk.getOrigin().x() + CHUNK_WIDTH_X - 1;

    for (int grid_x = near_x; grid_x <= far_x; grid_x++) {
        MyPlane plane = GetWestGridPlane(grid_x);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            MyGridCoord coord = WorldToGridCoord(impact, NUDGE_EAST);
            if (chunk.isCoordWithin(coord)) {
                const Block *block = chunk.getBlockGlobal_RO(coord);
                if (block->isFilled()) {
                    *pOut_coord    = coord;
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
    MyGridCoord *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int near_x = chunk.getOrigin().x() + CHUNK_WIDTH_X;
    int far_x  = chunk.getOrigin().x() + 1;

    for (int grid_x = near_x; grid_x >= far_x; grid_x--) {
        MyPlane plane = GetEastGridPlane(grid_x);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            MyGridCoord coord = WorldToGridCoord(impact, NUDGE_WEST);
            if (chunk.isCoordWithin(coord)) {
                const Block *block = chunk.getBlockGlobal_RO(coord);
                if (block->isFilled()) {
                    *pOut_coord    = coord;
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
    MyGridCoord *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int high_y = CHUNK_HEIGHT_Y;
    int low_y  = 1;

    for (int grid_y = high_y; grid_y >= low_y; grid_y--) {
        MyPlane plane = GetTopGridPlane(grid_y);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            MyGridCoord coord = WorldToGridCoord(impact, NUDGE_DOWN);
            if (chunk.isCoordWithin(coord)) {
                const Block *block = chunk.getBlockGlobal_RO(coord);
                if (block->isFilled()) {
                    *pOut_coord    = coord;
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
    MyGridCoord *pOut_coord, MyVec4 *pOut_impact, GLfloat *pOut_distance)
{
    int low_y  = 0;
    int high_y = CHUNK_HEIGHT_Y - 1;

    for (int grid_y = low_y; grid_y <= high_y; grid_y++) {
        MyPlane plane = GetBottomGridPlane(grid_y);

        MyVec4  impact;
        GLfloat distance;
        HitTestEnum hit = WorldHitTest(eye_ray, plane, &impact, &distance);
        if (hit == HITTEST_SUCCESS) {
            MyGridCoord coord = WorldToGridCoord(impact, NUDGE_UP);
            if (chunk.isCoordWithin(coord)) {
                const Block *block = chunk.getBlockGlobal_RO(coord);
                if (block->isFilled()) {
                    *pOut_coord    = coord;
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
    FaceEnum winner  = FACE_NONE;
    GLfloat  closest = FLT_MAX;

    // South faces.
    MyGridCoord south_coord;
    MyVec4  south_impact;
    GLfloat south_dist = 0.0f;
    bool south_hit = CheckSouthFaces(chunk, eye_ray, &south_coord, &south_impact, &south_dist);
    if (south_hit && (south_dist < closest)) {
        winner  = FACE_SOUTH;
        closest = south_dist;
    }

    // North faces.
    MyGridCoord north_coord;
    MyVec4  north_impact;
    GLfloat north_dist = 0.0f;
    bool north_hit = CheckNorthFaces(chunk, eye_ray, &north_coord, &north_impact, &north_dist);
    if (north_hit && (north_dist < closest)) {
        winner  = FACE_NORTH;
        closest = north_dist;
    }

    // West faces.
    MyGridCoord west_coord;
    MyVec4  west_impact;
    GLfloat west_dist = 0.0;
    bool west_hit = CheckWestFaces(chunk, eye_ray, &west_coord, &west_impact, &west_dist);
    if (west_hit && (west_dist < closest)) {
        winner  = FACE_WEST;
        closest = west_dist;
    }

    // East faces.
    MyGridCoord east_coord;
    MyVec4  east_impact;
    GLfloat east_dist = 0.0;
    bool east_hit = CheckEastFaces(chunk, eye_ray, &east_coord, &east_impact, &east_dist);
    if (east_hit && (east_dist < closest)) {
        winner  = FACE_EAST;
        closest = east_dist;
    }

    // Top faces.
    MyGridCoord top_coord;
    MyVec4  top_impact;
    GLfloat top_dist   = 0.0;
    bool top_hit = CheckTopFaces(chunk, eye_ray, &top_coord, &top_impact, &top_dist);
    if (top_hit && (top_dist < closest)) {
        winner  = FACE_TOP;
        closest = top_dist;
    }

    // Bottom faces.
    MyGridCoord bottom_coord;
    MyVec4  bottom_impact;
    GLfloat bottom_dist = 0.0;
    bool bottom_hit = CheckBottomFaces(chunk, eye_ray, &bottom_coord, &bottom_impact, &bottom_dist);
    if (bottom_hit && (bottom_dist < closest)) {
        winner  = FACE_BOTTOM;
        closest = bottom_dist;
    }

    // All done. Draw the winner.
    const ChunkOrigin &origin = chunk.getOrigin();

    switch (winner) {
    case FACE_NONE:
        // If we got to here, we didn't hit anything.
        return false;

    case FACE_SOUTH:
        *pOut = ChunkHitTestDetail(origin, south_coord, FACE_SOUTH, south_impact, south_dist);
        return true;

    case FACE_NORTH:
        *pOut = ChunkHitTestDetail(origin, north_coord, FACE_NORTH, north_impact, north_dist);
        return true;

    case FACE_WEST:
        *pOut = ChunkHitTestDetail(origin, west_coord, FACE_WEST, west_impact, west_dist);
        return true;

    case FACE_EAST:
        *pOut = ChunkHitTestDetail(origin, east_coord, FACE_EAST, east_impact, east_dist);
        return true;

    case FACE_TOP:
        *pOut = ChunkHitTestDetail(origin, top_coord, FACE_TOP, top_impact, top_dist);
        return true;

    case FACE_BOTTOM:
        *pOut = ChunkHitTestDetail(origin, bottom_coord, FACE_BOTTOM, bottom_impact, bottom_dist);
        return true;

    default:
        PrintTheImpossible(__FILE__, __LINE__, winner);
        return false;
    }
}


// The second call available outside this file.
void ChunkHitTestToQuad(const Chunk &chunk, const ChunkHitTestDetail &details, VertList_PT *pOut)
{
    MyGridCoord global_coord = details.getGlobalCoord();
    MyGridCoord local_coord  = chunk.globalToLocalCoord(details.getGlobalCoord());

    Brady brady(chunk, local_coord, details.getFace());

    pOut->clear();
    brady.addToVertList_PT(pOut);
    pOut->update();
}



#include "stdafx.h"
#include "chunk_stripe.h"
#include "common_util.h"

#include "brady.h"
#include "block.h"
#include "chunk.h"


// Get the type of a block.
BlockType ChunkStripe::getBlockType(int z) const
{
    assert((z >= 0) && (z < CHUNK_WIDTH));
    return m_blocks[z].getBlockType();
}


// Set the type of a block
void ChunkStripe::setBlockType(int z, BlockType block_type)
{
    assert((z >= 0) && (z < CHUNK_WIDTH));
    m_blocks[z].setBlockType(block_type);
}


// Given a block, recalc its surface.
BlockSurface ChunkStripe::calcSurfaceForBlock(int local_z, FaceEnum face)
{
    BlockType content = getBlockType(local_z);
    switch (content) {
    case BT_GRASS: return SURF_GRASS;
    case BT_DIRT:  return SURF_DIRT;
    case BT_STONE: return SURF_STONE;
    default:
        PrintTheImpossible(__FILE__, __LINE__, content);
        return SURF_NONE;
    }
}


// Recalc the exposed faces for one stripe.
// Return true if any of the blocks changed.
bool ChunkStripe::recalcExposures(const Chunk &owner, int local_x, int local_y)
{
    bool result = false;
    for (int z = 0; z < CHUNK_WIDTH; z++) {
        LocalGrid local_coord(local_x, local_y, z);
        if (recalcExposureForBlock(owner, local_coord)) {
            result = true;
        }
    }

    m_has_exposures = result;
    return result;
}


// Rebuild the vertex list for one stripe.
// If we don't have any exposed faces, we can skip this step.
void ChunkStripe::addToSurfaceLists(Chunk &owner, int local_x, int local_y)
{
    if (!m_has_exposures) {
        return;
    }

    for (int z = 0; z < CHUNK_WIDTH; z++) {
        LocalGrid local_coord(local_x, local_y, z);
        addVertsForBlock(owner, local_coord);
    }
}


// Figure out which faces of a block are exposed.
// Return true if we have any exposures for this block.
bool ChunkStripe::recalcExposureForBlock(const Chunk &owner, const LocalGrid local_coord)
{
    Block &current = m_blocks[local_coord.z()];

    current.clearExposureFlags();

    // If this block is empty, it doesn't generate any faces.
    if (IsBlockTypeEmpty(current.getBlockType())) {
        return false;
    }

    // Check for edge cases.
    int x = local_coord.x();
    int y = local_coord.y();
    int z = local_coord.z();

    bool west_edge = (x == 0);
    bool east_edge = (x == (CHUNK_WIDTH - 1));

    bool south_edge = (z == 0);
    bool north_edge = (z == (CHUNK_WIDTH - 1));

    bool bottom_edge = (y == 0);
    bool top_edge    = (y == (CHUNK_HEIGHT - 1));

    // Get our six neigbors, straddling across chunk boundaries if necessary.
    BlockType west_content = BT_AIR;
    if (west_edge) {
        const Chunk *neighbor = owner.getNeighborWest();
        if (neighbor != nullptr) {
            LocalGrid coord(CHUNK_WIDTH - 1, y, z);
            west_content = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x - 1, y, z);
        west_content = owner.getBlockType(coord);
    }

    BlockType east_content = BT_AIR;
    if (east_edge) {
        const Chunk *neighbor = owner.getNeighborEast();
        if (neighbor != nullptr) {
            LocalGrid coord(0, y, z);
            east_content = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x + 1, y, z);
        east_content = owner.getBlockType(coord);
    }

    BlockType south_content = BT_AIR;
    if (south_edge) {
        const Chunk *neighbor = owner.getNeighborSouth();
        if (neighbor != nullptr) {
            LocalGrid coord(x, y, CHUNK_WIDTH - 1);
            south_content = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x, y, z - 1);
        south_content = owner.getBlockType(coord);
    }

    BlockType north_content = BT_AIR;
    if (north_edge) {
        const Chunk *neighbor = owner.getNeighborNorth();
        if (neighbor != nullptr) {
            LocalGrid coord(x, y, 0);
            north_content = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x, y, z + 1);
        north_content = owner.getBlockType(coord);
    }

    BlockType top_content = BT_AIR;
    if (!top_edge) {
        LocalGrid coord(x, y + 1, z);
        top_content = owner.getBlockType(coord);
    }

    BlockType bottom_content = BT_AIR;
    if (!bottom_edge) {
        LocalGrid coord(x, y - 1, z);
        bottom_content = owner.getBlockType(coord);
    }

    // Check each of the faces.
    bool west_flag   = IsBlockTypeEmpty(west_content);
    bool east_flag   = IsBlockTypeEmpty(east_content);

    bool south_flag  = IsBlockTypeEmpty(south_content);
    bool north_flag  = IsBlockTypeEmpty(north_content);

    bool top_flag    = IsBlockTypeEmpty(top_content);
    bool bottom_flag = IsBlockTypeEmpty(bottom_content);

    current.setExposure(FACE_SOUTH,  south_flag);
    current.setExposure(FACE_NORTH,  north_flag);
    current.setExposure(FACE_WEST,   west_flag);
    current.setExposure(FACE_EAST,   east_flag);
    current.setExposure(FACE_TOP,    top_flag);
    current.setExposure(FACE_BOTTOM, bottom_flag);

    return current.hasExposures();
}


// Add the quads for a block.
void ChunkStripe::addVertsForBlock(Chunk &owner, const LocalGrid &local_coord)
{
    const Block &current = m_blocks[local_coord.z()];

    // If this block is empty, it doesn't generate any faces.
    if (IsBlockTypeEmpty(current.getBlockType())) {
        return;
    }

    int x = local_coord.x();
    int y = local_coord.y();
    int z = local_coord.z();

    // Top face.
    if (current.getExposure(FACE_TOP)) {
        Brady brady(owner, local_coord, FACE_TOP);
        BlockSurface surf = calcSurfaceForBlock(z, FACE_TOP);
        VertList_PNT *pList = owner.getSurfaceList(surf);
        brady.addToVertList_PNT(pList);
    }

    // Bottom face.
    if (current.getExposure(FACE_BOTTOM)) {
        Brady brady(owner, local_coord, FACE_BOTTOM);
        BlockSurface surf = calcSurfaceForBlock(z, FACE_BOTTOM);
        VertList_PNT *pList = owner.getSurfaceList(surf);
        brady.addToVertList_PNT(pList);
    }

    // Southern face.
    if (current.getExposure(FACE_SOUTH)) {
        Brady brady(owner, local_coord, FACE_SOUTH);
        BlockSurface surf = calcSurfaceForBlock(z, FACE_SOUTH);
        VertList_PNT *pList = owner.getSurfaceList(surf);
        brady.addToVertList_PNT(pList);
    }

    // Northern face.
    if (current.getExposure(FACE_NORTH)) {
        Brady brady(owner, local_coord, FACE_NORTH);
        BlockSurface surf = calcSurfaceForBlock(z, FACE_NORTH);
        VertList_PNT *pList = owner.getSurfaceList(surf);
        brady.addToVertList_PNT(pList);
    }

    // Eastern face.
    if (current.getExposure(FACE_EAST)) {
        Brady brady(owner, local_coord, FACE_EAST);
        BlockSurface surf = calcSurfaceForBlock(z, FACE_EAST);
        VertList_PNT *pList = owner.getSurfaceList(surf);
        brady.addToVertList_PNT(pList);
    }

    // Western face.
    if (current.getExposure(FACE_WEST)) {
        Brady brady(owner, local_coord, FACE_WEST);
        BlockSurface surf = calcSurfaceForBlock(z, FACE_WEST);
        VertList_PNT *pList = owner.getSurfaceList(surf);
        brady.addToVertList_PNT(pList);
    }
}


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


// Recalc the exposed faces for one stripe.
// Return true if any of the blocks changed.
SurfaceTotals ChunkStripe::recalcExposures(const Chunk &owner, int local_x, int local_y)
{
    m_surface_totals.reset();

    for (int z = 0; z < CHUNK_WIDTH; z++) {
        LocalGrid local_coord(local_x, local_y, z);
        m_surface_totals.add(recalcExposureForBlock(owner, local_coord));
    }

    return m_surface_totals;
}


// Rebuild the vertex list for one stripe.
// If we don't have any exposed faces, we can skip this step.
void ChunkStripe::addToSurfaceLists(Chunk &owner, int local_x, int local_y)
{
    if (!m_surface_totals.hasAnything()) {
        return;
    }

    for (int z = 0; z < CHUNK_WIDTH; z++) {
        LocalGrid local_coord(local_x, local_y, z);
        addVertsForBlock(owner, local_coord);
    }
}


// Figure out which faces of a block are exposed.
// Return a surface totals object, showing what we added.
SurfaceTotals ChunkStripe::recalcExposureForBlock(const Chunk &owner, const LocalGrid local_coord)
{
    SurfaceTotals totals;

    Block &current = m_blocks[local_coord.z()];

    current.clearSurfaces();

    // If this block is empty, it doesn't generate any faces.
    BlockType block_type = current.getBlockType();
    if (IsBlockTypeEmpty(block_type)) {
        return totals;
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
    BlockType west_block_type = BT_AIR;
    if (west_edge) {
        const Chunk *neighbor = owner.getNeighborWest();
        if (neighbor != nullptr) {
            LocalGrid coord(CHUNK_WIDTH - 1, y, z);
            west_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x - 1, y, z);
        west_block_type = owner.getBlockType(coord);
    }

    BlockType east_block_type = BT_AIR;
    if (east_edge) {
        const Chunk *neighbor = owner.getNeighborEast();
        if (neighbor != nullptr) {
            LocalGrid coord(0, y, z);
            east_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x + 1, y, z);
        east_block_type = owner.getBlockType(coord);
    }

    BlockType south_block_type = BT_AIR;
    if (south_edge) {
        const Chunk *neighbor = owner.getNeighborSouth();
        if (neighbor != nullptr) {
            LocalGrid coord(x, y, CHUNK_WIDTH - 1);
            south_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x, y, z - 1);
        south_block_type = owner.getBlockType(coord);
    }

    BlockType north_block_type = BT_AIR;
    if (north_edge) {
        const Chunk *neighbor = owner.getNeighborNorth();
        if (neighbor != nullptr) {
            LocalGrid coord(x, y, 0);
            north_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x, y, z + 1);
        north_block_type = owner.getBlockType(coord);
    }

    BlockType top_block_type = BT_AIR;
    if (!top_edge) {
        LocalGrid coord(x, y + 1, z);
        top_block_type = owner.getBlockType(coord);
    }

    BlockType bottom_block_type = BT_AIR;
    if (!bottom_edge) {
        LocalGrid coord(x, y - 1, z);
        bottom_block_type = owner.getBlockType(coord);
    }

    // Check each of the faces.
    SurfaceType west_surface   = CalcSurfaceType(block_type, FACE_WEST, west_block_type);
    SurfaceType east_surface   = CalcSurfaceType(block_type, FACE_EAST, east_block_type);

    SurfaceType south_surface  = CalcSurfaceType(block_type, FACE_SOUTH, south_block_type);
    SurfaceType north_surface  = CalcSurfaceType(block_type, FACE_NORTH, north_block_type);

    SurfaceType top_surface    = CalcSurfaceType(block_type, FACE_TOP,    top_block_type);
    SurfaceType bottom_surface = CalcSurfaceType(block_type, FACE_BOTTOM, bottom_block_type);

    current.setSurface(FACE_SOUTH,  south_surface);
    current.setSurface(FACE_NORTH,  north_surface);
    current.setSurface(FACE_WEST,   west_surface);
    current.setSurface(FACE_EAST,   east_surface);
    current.setSurface(FACE_TOP,    top_surface);
    current.setSurface(FACE_BOTTOM, bottom_surface);

    return current.getSurfaceTotals();
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
    SurfaceType top_surf = current.getSurface(FACE_TOP);
    if (top_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_TOP);
        VertList_PNT *pList = owner.getSurfaceList(top_surf);
        brady.addToVertList_PNT(pList);
    }

    // Bottom face.
    SurfaceType bottom_surf = current.getSurface(FACE_BOTTOM);
    if (bottom_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_BOTTOM);
        VertList_PNT *pList = owner.getSurfaceList(bottom_surf);
        brady.addToVertList_PNT(pList);
    }

    // Southern face.
    SurfaceType south_surf = current.getSurface(FACE_SOUTH);
    if (south_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_SOUTH);
        VertList_PNT *pList = owner.getSurfaceList(south_surf);
        brady.addToVertList_PNT(pList);
    }

    // Northern face.
    SurfaceType north_surf = current.getSurface(FACE_NORTH);
    if (north_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_NORTH);
        VertList_PNT *pList = owner.getSurfaceList(north_surf);
        brady.addToVertList_PNT(pList);
    }

    // Eastern face.
    SurfaceType east_surf = current.getSurface(FACE_EAST);
    if (east_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_EAST);
        VertList_PNT *pList = owner.getSurfaceList(east_surf);
        brady.addToVertList_PNT(pList);
    }

    // Western face.
    SurfaceType west_surf = current.getSurface(FACE_WEST);
    if (west_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_WEST);
        VertList_PNT *pList = owner.getSurfaceList(west_surf);
        brady.addToVertList_PNT(pList);
    }
}

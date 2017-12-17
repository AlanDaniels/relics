
#include "stdafx.h"
#include "chunk_stripe.h"
#include "common_util.h"

#include "brady.h"
#include "block.h"
#include "chunk.h"

// Surface totals ctor.
SurfaceTotals::SurfaceTotals()
{
    for (int i = 0; i < SURF_MAX_COUNT; i++) {
        m_counts[i] = 0;
    }
}


// Increment our surface type.
void SurfaceTotals::increment(SurfaceType surf)
{
    if (surf != SURF_NOTHING) {
        assert(surf < SURF_MAX_COUNT);
        m_counts[surf]++;
    }
}


// Get the count for one surface.
int SurfaceTotals::get(SurfaceType surf) const
{
    assert(surf < SURF_MAX_COUNT);
    return m_counts[surf];
}


// Get the count for all surfaces.
int SurfaceTotals::getGrandTotal() const
{
    int result = 0;
    for (int i = 0; i < SURF_MAX_COUNT; i++) {
        result += m_counts[i];
    }

    return result;
}


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
void ChunkStripe::recalcAllExposures(const Chunk &owner, int local_x, int local_y, SurfaceTotals *pOut)
{
    bool anything = false;

    for (int z = 0; z < CHUNK_WIDTH; z++) {
        LocalGrid local_coord(local_x, local_y, z);
        if (recalcExposureForBlock(owner, local_coord, pOut)) {
            anything = true;
        }
    }

    m_has_exposures = anything;
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
// Return a surface totals object, showing what we added.
bool ChunkStripe::recalcExposureForBlock(
    const Chunk &owner, const LocalGrid local_coord, SurfaceTotals *pOut)
{
    Block &current = m_blocks[local_coord.z()];

    current.clearSurfaces();

    // If this block is empty, it doesn't generate any faces.
    BlockType block_type = current.getBlockType();
    if (IsBlockTypeEmpty(block_type)) {
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
    SurfaceType west_surf   = CalcSurfaceType(block_type, FACE_WEST, west_block_type);
    SurfaceType east_surf   = CalcSurfaceType(block_type, FACE_EAST, east_block_type);

    SurfaceType south_surf  = CalcSurfaceType(block_type, FACE_SOUTH, south_block_type);
    SurfaceType north_surf  = CalcSurfaceType(block_type, FACE_NORTH, north_block_type);

    SurfaceType top_surf    = CalcSurfaceType(block_type, FACE_TOP,    top_block_type);
    SurfaceType bottom_surf = CalcSurfaceType(block_type, FACE_BOTTOM, bottom_block_type);

    current.setSurface(FACE_SOUTH,  south_surf);
    current.setSurface(FACE_NORTH,  north_surf);
    current.setSurface(FACE_WEST,   west_surf);
    current.setSurface(FACE_EAST,   east_surf);
    current.setSurface(FACE_TOP,    top_surf);
    current.setSurface(FACE_BOTTOM, bottom_surf);

    pOut->increment(south_surf);
    pOut->increment(north_surf);
    pOut->increment(west_surf);
    pOut->increment(east_surf);
    pOut->increment(top_surf);
    pOut->increment(bottom_surf);

    // Return true if anything we calculated has exposures.
    return (
        (south_surf  != SURF_NOTHING) ||
        (north_surf  != SURF_NOTHING) ||
        (west_surf   != SURF_NOTHING) ||
        (east_surf   != SURF_NOTHING) ||
        (top_surf    != SURF_NOTHING) ||
        (bottom_surf != SURF_NOTHING));
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
        VertList_PNT &list = owner.getSurfaceListForWriting(top_surf);
        brady.addToVertList_PNT(&list);
    }

    // Bottom face.
    SurfaceType bottom_surf = current.getSurface(FACE_BOTTOM);
    if (bottom_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_BOTTOM);
        VertList_PNT &list = owner.getSurfaceListForWriting(bottom_surf);
        brady.addToVertList_PNT(&list);
    }

    // Southern face.
    SurfaceType south_surf = current.getSurface(FACE_SOUTH);
    if (south_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_SOUTH);
        VertList_PNT &list = owner.getSurfaceListForWriting(south_surf);
        brady.addToVertList_PNT(&list);
    }

    // Northern face.
    SurfaceType north_surf = current.getSurface(FACE_NORTH);
    if (north_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_NORTH);
        VertList_PNT &list = owner.getSurfaceListForWriting(north_surf);
        brady.addToVertList_PNT(&list);
    }

    // Eastern face.
    SurfaceType east_surf = current.getSurface(FACE_EAST);
    if (east_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_EAST);
        VertList_PNT &list = owner.getSurfaceListForWriting(east_surf);
        brady.addToVertList_PNT(&list);
    }

    // Western face.
    SurfaceType west_surf = current.getSurface(FACE_WEST);
    if (west_surf != SURF_NOTHING) {
        Brady brady(owner, local_coord, FACE_WEST);
        VertList_PNT &list = owner.getSurfaceListForWriting(west_surf);
        brady.addToVertList_PNT(&list);
    }
}

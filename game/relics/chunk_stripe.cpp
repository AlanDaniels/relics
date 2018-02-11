
#include "stdafx.h"
#include "chunk_stripe.h"
#include "common_util.h"

#include "add_face.h"
#include "block.h"
#include "chunk.h"

// Surface totals ctor.
SurfaceTotals::SurfaceTotals()
{
    for (int i = 0; i < SURFACE_TYPE_COUNT; i++) {
        m_counts[i] = 0;
    }
}


// Increment our surface type.
void SurfaceTotals::increment(SurfaceType surf)
{
    if (surf != SurfaceType::NOTHING) {
        int index = static_cast<int>(surf);
        assert(index < SURFACE_TYPE_COUNT);
        m_counts[index]++;
    }
}


// Get the count for one surface.
int SurfaceTotals::get(SurfaceType surf) const
{
    int index = static_cast<int>(surf);
    assert(index < SURFACE_TYPE_COUNT);
    return m_counts.at(index);
}


// Get the count for all surfaces.
int SurfaceTotals::getGrandTotal() const
{
    int result = 0;
    for (int i = 0; i < SURFACE_TYPE_COUNT; i++) {
        result += m_counts.at(i);
    }

    return result;
}


// Get the type of a block.
BlockType ChunkStripe::getBlockType(int z) const
{
    assert((z >= 0) && (z < CHUNK_WIDTH));
    return m_blocks.at(z).getBlockType();
}


// Set the type of a block
void ChunkStripe::setBlockType(int z, BlockType block_type)
{
    assert((z >= 0) && (z < CHUNK_WIDTH));
    m_blocks.at(z).setBlockType(block_type);
}


// Figure out which faces of a block are exposed.
// Populate a surface totals object, showing what we added.
// Return if this block has any exposures at all.
bool ChunkStripe::recalcExposuresForBlock(
    const Chunk &chunk, const LocalGrid &local_coord, SurfaceTotals *pOut)
{
    Block &current = m_blocks.at(local_coord.z());

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
    BlockType west_block_type = BlockType::AIR;
    if (west_edge) {
        const Chunk *neighbor = chunk.getNeighborWest();
        if (neighbor != nullptr) {
            LocalGrid coord(CHUNK_WIDTH - 1, y, z);
            west_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x - 1, y, z);
        west_block_type = chunk.getBlockType(coord);
    }

    BlockType east_block_type = BlockType::AIR;
    if (east_edge) {
        const Chunk *neighbor = chunk.getNeighborEast();
        if (neighbor != nullptr) {
            LocalGrid coord(0, y, z);
            east_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x + 1, y, z);
        east_block_type = chunk.getBlockType(coord);
    }

    BlockType south_block_type = BlockType::AIR;
    if (south_edge) {
        const Chunk *neighbor = chunk.getNeighborSouth();
        if (neighbor != nullptr) {
            LocalGrid coord(x, y, CHUNK_WIDTH - 1);
            south_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x, y, z - 1);
        south_block_type = chunk.getBlockType(coord);
    }

    BlockType north_block_type = BlockType::AIR;
    if (north_edge) {
        const Chunk *neighbor = chunk.getNeighborNorth();
        if (neighbor != nullptr) {
            LocalGrid coord(x, y, 0);
            north_block_type = neighbor->getBlockType(coord);
        }
    }
    else {
        LocalGrid coord(x, y, z + 1);
        north_block_type = chunk.getBlockType(coord);
    }

    BlockType top_block_type = BlockType::AIR;
    if (!top_edge) {
        LocalGrid coord(x, y + 1, z);
        top_block_type = chunk.getBlockType(coord);
    }

    BlockType bottom_block_type = BlockType::AIR;
    if (!bottom_edge) {
        LocalGrid coord(x, y - 1, z);
        bottom_block_type = chunk.getBlockType(coord);
    }

    // Check each of the faces.
    SurfaceType west_surf   = CalcSurfaceType(block_type, FaceType::WEST, west_block_type);
    SurfaceType east_surf   = CalcSurfaceType(block_type, FaceType::EAST, east_block_type);

    SurfaceType south_surf  = CalcSurfaceType(block_type, FaceType::SOUTH, south_block_type);
    SurfaceType north_surf  = CalcSurfaceType(block_type, FaceType::NORTH, north_block_type);

    SurfaceType top_surf    = CalcSurfaceType(block_type, FaceType::TOP,    top_block_type);
    SurfaceType bottom_surf = CalcSurfaceType(block_type, FaceType::BOTTOM, bottom_block_type);

    current.setSurface(FaceType::SOUTH,  south_surf);
    current.setSurface(FaceType::NORTH,  north_surf);
    current.setSurface(FaceType::WEST,   west_surf);
    current.setSurface(FaceType::EAST,   east_surf);
    current.setSurface(FaceType::TOP,    top_surf);
    current.setSurface(FaceType::BOTTOM, bottom_surf);

    pOut->increment(south_surf);
    pOut->increment(north_surf);
    pOut->increment(west_surf);
    pOut->increment(east_surf);
    pOut->increment(top_surf);
    pOut->increment(bottom_surf);

    // Return true if anything we calculated has exposures.
    return (
        (south_surf  != SurfaceType::NOTHING) ||
        (north_surf  != SurfaceType::NOTHING) ||
        (west_surf   != SurfaceType::NOTHING) ||
        (east_surf   != SurfaceType::NOTHING) ||
        (top_surf    != SurfaceType::NOTHING) ||
        (bottom_surf != SurfaceType::NOTHING));
}


// Add the quads for a block.
void ChunkStripe::addToSurfaceLists(Chunk &owner, const LocalGrid &local_coord)
{
    const Block &current = m_blocks.at(local_coord.z());

    // If this block is empty, it doesn't generate any faces.
    if (IsBlockTypeEmpty(current.getBlockType())) {
        return;
    }

    int x = local_coord.x();
    int y = local_coord.y();
    int z = local_coord.z();

    // Top face.
    SurfaceType top_surf = current.getSurface(FaceType::TOP);
    if (top_surf != SurfaceType::NOTHING) {
        VertList_PNT &list = owner.getSurfaceListForWriting(top_surf);
        auto tris = GetLandscapePatch_PNT(owner, local_coord, FaceType::TOP);
        list.add(tris.data(), tris.size());
    }

    // Bottom face.
    SurfaceType bottom_surf = current.getSurface(FaceType::BOTTOM);
    if (bottom_surf != SurfaceType::NOTHING) {
        VertList_PNT &list = owner.getSurfaceListForWriting(bottom_surf);
        auto tris = GetLandscapePatch_PNT(owner, local_coord, FaceType::BOTTOM);
        list.add(tris.data(), tris.size());
    }

    // Southern face.
    SurfaceType south_surf = current.getSurface(FaceType::SOUTH);
    if (south_surf != SurfaceType::NOTHING) {
        VertList_PNT &list = owner.getSurfaceListForWriting(south_surf);
        auto tris = GetLandscapePatch_PNT(owner, local_coord, FaceType::SOUTH);
        list.add(tris.data(), tris.size());
    }

    // Northern face.
    SurfaceType north_surf = current.getSurface(FaceType::NORTH);
    if (north_surf != SurfaceType::NOTHING) {
        VertList_PNT &list = owner.getSurfaceListForWriting(north_surf);
        auto tris = GetLandscapePatch_PNT(owner, local_coord, FaceType::NORTH);
        list.add(tris.data(), tris.size());
    }

    // Eastern face.
    SurfaceType east_surf = current.getSurface(FaceType::EAST);
    if (east_surf != SurfaceType::NOTHING) {
        VertList_PNT &list = owner.getSurfaceListForWriting(east_surf);
        auto tris = GetLandscapePatch_PNT(owner, local_coord, FaceType::EAST);
        list.add(tris.data(), tris.size());
    }

    // Western face.
    SurfaceType west_surf = current.getSurface(FaceType::WEST);
    if (west_surf != SurfaceType::NOTHING) {
        VertList_PNT &list = owner.getSurfaceListForWriting(west_surf);
        auto tris = GetLandscapePatch_PNT(owner, local_coord, FaceType::WEST);
        list.add(tris.data(), tris.size());
    }
}

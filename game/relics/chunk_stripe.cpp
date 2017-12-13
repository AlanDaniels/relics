
#include "stdafx.h"
#include "chunk_stripe.h"
#include "common_util.h"

#include "brady.h"
#include "block.h"
#include "chunk.h"


// Get a block, read-only version.
const Block *ChunkStripe::getBlock_RO(int z) const
{
    assert((z >= 0) && (z < CHUNK_WIDTH));
    return &m_blocks[z];
}


// Get a block, writeable version.
Block *ChunkStripe::getBlock_RW(int z)
{
    assert((z >= 0) && (z < CHUNK_WIDTH));
    return &m_blocks[z];
}


// Populate a stripe with a particular block type.
void ChunkStripe::fillStripe(BlockContent block_type)
{
    for (int z = 0; z < CHUNK_WIDTH; z++) {
        m_blocks[z].setContent(block_type);
    }
}


// Given a block, recalc its surface.
BlockSurface ChunkStripe::calcSurfaceForBlock(int local_z, FaceEnum face)
{
    const Block *block = getBlock_RO(local_z);

    BlockContent content = block->getContent();
    switch (content) {
    case CONTENT_GRASS:   return SURF_GRASS;
    case CONTENT_DIRT:    return SURF_DIRT;
    case CONTENT_STONE:   return SURF_STONE;
    case CONTENT_BEDROCK: return SURF_BEDROCK;
    default:
        PrintTheImpossible(__FILE__, __LINE__, content);
        return SURF_NONE;
    }
}


// Recalc the exposed faces for one stripe.
// Return true if any of the blocks changed.
void ChunkStripe::recalcExposures()
{
    for (int z = 0; z < CHUNK_WIDTH; z++) {
        recalcExposureForBlock(z);
    }
}


// Rebuild the vertex list for one stripe.
// Return the total number of faces added.
void ChunkStripe::addToVertLists(ChunkVertLists *pOut)
{
    for (int local_z = 0; local_z < CHUNK_WIDTH; local_z++) {
        addVertsForBlock(pOut, local_z);
    }
}


// Figure out which faces of a block are exposed.
// Return true if the faces changed.
void ChunkStripe::recalcExposureForBlock(int local_z)
{
    assert((local_z >= 0) && (local_z < CHUNK_WIDTH));
    Block &current = m_blocks[local_z];

    current.clearExposureFlags();

    // If this block is empty, it doesn't generate any faces.
    if (current.isEmpty()) {
        return;
    }

    // Check for edge cases.
    int x = m_local_x;
    int y = m_local_y;
    int z = local_z;

    bool west_edge = (x == 0);
    bool east_edge = (x == (CHUNK_WIDTH - 1));

    bool south_edge = (z == 0);
    bool north_edge = (z == (CHUNK_WIDTH - 1));

    bool bottom_edge = (y == 0);
    bool top_edge    = (y == (CHUNK_HEIGHT - 1));

    // Get our six neigbors, straddling across chunk boundaries if necessary.
    BlockContent west_content = CONTENT_AIR;
    if (west_edge) {
        const Chunk *neighbor = m_owner->getNeighborWest();
        if (neighbor != nullptr) {
            const Block *block = neighbor->getBlockLocal_RO(CHUNK_WIDTH - 1, y, z);
            west_content = block->getContent();
        }
    }
    else {
        const Block *block = m_owner->getBlockLocal_RO(x - 1, y, z);
        west_content = block->getContent();
    }

    BlockContent east_content = CONTENT_AIR;
    if (east_edge) {
        const Chunk *neighbor = m_owner->getNeighborEast();
        if (neighbor != nullptr) {
            const Block * block = neighbor->getBlockLocal_RO(0, y, z);
            east_content = block->getContent();
        }
    }
    else {
        const Block *block = m_owner->getBlockLocal_RO(x + 1, y, z);
        east_content = block->getContent();
    }

    BlockContent south_content = CONTENT_AIR;
    if (south_edge) {
        const Chunk *neighbor = m_owner->getNeighborSouth();
        if (neighbor != nullptr) {
            const Block *block = neighbor->getBlockLocal_RO(x, y, CHUNK_WIDTH - 1);
            south_content = block->getContent();
        }
    }
    else {
        const Block *block = m_owner->getBlockLocal_RO(x, y, z - 1);
        south_content = block->getContent();
    }

    BlockContent north_content = CONTENT_AIR;
    if (north_edge) {
        const Chunk *neighbor = m_owner->getNeighborNorth();
        if (neighbor != nullptr) {
            const Block *block = neighbor->getBlockLocal_RO(x, y, 0);
            north_content = block->getContent();
        }
    }
    else {
        const Block *block = m_owner->getBlockLocal_RO(x, y, z + 1);
        north_content = block->getContent();
    }

    BlockContent top_content = top_edge ?
        CONTENT_AIR : 
        m_owner->getBlockLocal_RO(x, y + 1, z)->getContent();

    BlockContent bottom_content = bottom_edge ?
        CONTENT_AIR : 
        m_owner->getBlockLocal_RO(x, y - 1, z)->getContent();

    // Check each of the faces.
    bool west_flag  = IsContentEmpty(west_content);
    bool east_flag  = IsContentEmpty(east_content);

    bool south_flag  = IsContentEmpty(south_content);
    bool north_flag  = IsContentEmpty(north_content);

    bool top_flag    = IsContentEmpty(top_content);
    bool bottom_flag = IsContentEmpty(bottom_content);

    current.setExposure(FACE_SOUTH,  south_flag);
    current.setExposure(FACE_NORTH,  north_flag);
    current.setExposure(FACE_WEST,   west_flag);
    current.setExposure(FACE_EAST,   east_flag);
    current.setExposure(FACE_TOP,    top_flag);
    current.setExposure(FACE_BOTTOM, bottom_flag);
}


// Add the quads for a block.
void ChunkStripe::addVertsForBlock(ChunkVertLists *pOut, int local_z)
{
    const Block &current = m_blocks[local_z];

    // If this block is empty, it doesn't generate any faces.
    if (current.isEmpty()) {
        return;
    }

    int x = m_local_x;
    int y = m_local_y;
    int z = local_z;

    LocalGrid local_coord(x, y, z);

    // Top face.
    if (current.getExposure(FACE_TOP)) {
        Brady brady(*m_owner, local_coord, FACE_TOP);
        BlockSurface surf = calcSurfaceForBlock(local_z, FACE_TOP);
        VertList_PNT *pList = pOut->get(surf);
        brady.addToVertList_PNT(pList);
    }

    // Bottom face.
    if (current.getExposure(FACE_BOTTOM)) {
        Brady brady(*m_owner, local_coord, FACE_BOTTOM);
        BlockSurface surf = calcSurfaceForBlock(local_z, FACE_BOTTOM);
        VertList_PNT *pList = pOut->get(surf);
        brady.addToVertList_PNT(pList);
    }

    // Southern face.
    if (current.getExposure(FACE_SOUTH)) {
        Brady brady(*m_owner, local_coord, FACE_SOUTH);
        BlockSurface surf = calcSurfaceForBlock(local_z, FACE_SOUTH);
        VertList_PNT *pList = pOut->get(surf);
        brady.addToVertList_PNT(pList);
    }

    // Northern face.
    if (current.getExposure(FACE_NORTH)) {
        Brady brady(*m_owner, local_coord, FACE_NORTH);
        BlockSurface surf = calcSurfaceForBlock(local_z, FACE_NORTH);
        VertList_PNT *pList = pOut->get(surf);
        brady.addToVertList_PNT(pList);
    }

    // Eastern face.
    if (current.getExposure(FACE_EAST)) {
        Brady brady(*m_owner, local_coord, FACE_EAST);
        BlockSurface surf = calcSurfaceForBlock(local_z, FACE_EAST);
        VertList_PNT *pList = pOut->get(surf);
        brady.addToVertList_PNT(pList);
    }

    // Western face.
    if (current.getExposure(FACE_WEST)) {
        Brady brady(*m_owner, local_coord, FACE_WEST);
        BlockSurface surf = calcSurfaceForBlock(local_z, FACE_WEST);
        VertList_PNT *pList = pOut->get(surf);
        brady.addToVertList_PNT(pList);
    }
}

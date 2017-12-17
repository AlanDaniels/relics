#pragma once

#include "stdafx.h"
#include "block.h"
#include "draw_state_pct.h"
#include "utils.h"


class Chunk;
class ChunkVertLists;

// As we recalc exposures, keep track of how many surfaces we discover.
class SurfaceTotals
{
public:
    SurfaceTotals();
    void increment(SurfaceType surf_type);
    int  get(SurfaceType surf) const;

private:
    std::array<int, SURF_MAX_COUNT> m_counts;
};


// An array of blocks along the Z axis.
class ChunkStripe
{
public:
    ChunkStripe() : m_has_exposures(false) {}
    ~ChunkStripe() {}

    BlockType getBlockType(int local_z) const;
    void setBlockType(int local_z, BlockType block_type);

    void recalcAllExposures(const Chunk &owner, int local_x, int local_y, SurfaceTotals *pOut);
    void addToSurfaceLists(Chunk &owner, int local_x, int local_y);

private:
    // Disallow moving and copying.
    ChunkStripe(const ChunkStripe &that) = delete;
    void operator=(const ChunkStripe &that) = delete;
    ChunkStripe(ChunkStripe &&that) = delete;
    void operator=(ChunkStripe &&that) = delete;

    // Private methods.
    bool recalcExposureForBlock(const Chunk &chunk, const LocalGrid local_coord, SurfaceTotals *pOut);
    void addVertsForBlock(Chunk &owner, const LocalGrid &local_coord);

    // Private data.
    bool m_has_exposures;
    std::array<Block, CHUNK_WIDTH> m_blocks;
};

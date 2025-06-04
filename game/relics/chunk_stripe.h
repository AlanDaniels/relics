#pragma once

#include "stdafx.h"
#include "block.h"
#include "draw_state_pct.h"
#include "utils.h"


class Chunk;
class ChunkVertLists;
class Landscape;


// As we recalc exposures, keep track of how many surfaces we discover.
class SurfaceTotals
{
public:
    SurfaceTotals();
    void increment(SurfaceType surf_type);
    int  get(SurfaceType surf) const;
    int  getGrandTotal() const;

    DEFAULT_MOVING(SurfaceTotals)

private:
    FORBID_COPYING(SurfaceTotals)

    std::array<int, SURFACE_TYPE_COUNT> m_counts;
};


// An array of blocks along the Z axis.
class ChunkStripe
{
public:
    ChunkStripe() : 
        m_has_exposures(false) {}

    ~ChunkStripe() {}

    BlockType getBlockType(int local_z) const;
    void setBlockType(int local_z, BlockType block_type);

    bool recalcExposuresForBlock(const Chunk &chunk, const LocalGrid &local_coord, SurfaceTotals *pOut);
    void addToSurfaceLists(Chunk &chunk, const LocalGrid &local_coord);

private:
    FORBID_COPYING(ChunkStripe)
    FORBID_MOVING(ChunkStripe)

    // Private data.
    bool m_has_exposures;
    std::array<Block, CHUNK_WIDTH> m_blocks;
};
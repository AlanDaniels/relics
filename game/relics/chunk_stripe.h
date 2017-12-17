#pragma once

#include "stdafx.h"
#include "block.h"
#include "draw_state_pct.h"
#include "utils.h"


class Chunk;
class ChunkVertLists;


class ChunkStripe
{
public:
    ChunkStripe() :
        m_has_exposures(false) {}
    ~ChunkStripe() {}

    BlockType getBlockType(int local_z) const;
    void setBlockType(int local_z, BlockType block_type);

    bool recalcExposures(const Chunk &owner, int local_x, int local_y);
    void addToSurfaceLists(Chunk &owner, int local_x, int local_y);

private:
    // Disallow moving and copying.
    ChunkStripe(const ChunkStripe &that) = delete;
    void operator=(const ChunkStripe &that) = delete;
    ChunkStripe(ChunkStripe &&that) = delete;
    void operator=(ChunkStripe &&that) = delete;

    BlockSurface calcSurfaceForBlock(int local_z, FaceEnum face);
    bool recalcExposureForBlock(const Chunk &chunk, const LocalGrid local_coord);
    void addVertsForBlock(Chunk &owner, const LocalGrid &local_coord);

    bool m_has_exposures;
    std::array<Block, CHUNK_WIDTH> m_blocks;
};

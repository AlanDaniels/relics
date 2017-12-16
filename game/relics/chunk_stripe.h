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
    ChunkStripe() {}
    ~ChunkStripe() {}

    BlockType getBlockType(int local_z) const;
    void setBlockType(int local_z, BlockType block_type);

    void recalcExposures(const Chunk &owner, int local_x, int local_y);
    void addToVertLists(const Chunk &owner, int local_x, int local_y, ChunkVertLists *pOut);

private:
    // Disallow moving and copying.
    ChunkStripe(const ChunkStripe &that) = delete;
    void operator=(const ChunkStripe &that) = delete;
    ChunkStripe(ChunkStripe &&that) = delete;
    void operator=(ChunkStripe &&that) = delete;

    BlockSurface calcSurfaceForBlock(int local_z, FaceEnum face);
    void recalcExposureForBlock(const Chunk &chunk, const LocalGrid local_coord);
    void addVertsForBlock(const Chunk &owner, const LocalGrid &local_coord, ChunkVertLists *pOut);

    std::array<Block, CHUNK_WIDTH> m_blocks;
};

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
    ChunkStripe(const Chunk *owner, int local_x, int local_y) :
        m_owner(owner),
        m_local_x(local_x), 
        m_local_y(local_y) {}

    const Block *getBlock_RO(int local_z) const;
    Block *getBlock_RW(int local_z);

    void fillStripe(BlockType block_type);
    void recalcExposures();
    void addToVertLists(ChunkVertLists *pOut);

private:
    // Disallow copying, and default ctor.
    ChunkStripe() = delete;
    ChunkStripe(const ChunkStripe &that) = delete;
    void operator=(const ChunkStripe &that) = delete;

    BlockSurface calcSurfaceForBlock(int local_z, FaceEnum face);
    void recalcExposureForBlock(int local_z);
    void addVertsForBlock(ChunkVertLists *pOut, int local_z);

    const Chunk *m_owner;
    int m_local_x;
    int m_local_y;

    Block m_blocks[CHUNK_WIDTH];
};

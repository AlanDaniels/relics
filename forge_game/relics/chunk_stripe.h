#pragma once

#include "stdafx.h"
#include "block.h"
#include "draw_state_pct.h"
#include "utils.h"


class Chunk;
class LandscapeVertLists;


class ChunkStripe
{
public:
    ChunkStripe(const Chunk *pOwner, int local_x, int local_y) :
        m_pOwner(pOwner), m_local_x(local_x), m_local_y(local_y) {}

    const Block *getBlock_RO(int local_z) const;
    Block *getBlock_RW(int local_z);

    void fillStripe(BlockContent block_type);

    void recalcExposuresInterior();
    void recalcExposuresAll();

    void addToVertLists(LandscapeVertLists *pOut);

private:
    // Disallow copying, and default ctor.
    ChunkStripe() = delete;
    ChunkStripe(const ChunkStripe &that) = delete;
    void operator=(const ChunkStripe &that) = delete;

    BlockSurface calcSurfaceForBlock(int local_z, FaceEnum face);
    void recalcExposureForBlock(int local_z);
    void addVertsForBlock(LandscapeVertLists *pOut, int local_z);

    const Chunk *m_pOwner;
    int m_local_x;
    int m_local_y;

    Block m_blocks[CHUNK_DEPTH_Z];
};

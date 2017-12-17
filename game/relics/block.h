#pragma once

#include "stdafx.h"
#include "draw_state_pt.h"
#include "utils.h"

class Chunk;


enum BlockType
{
    BT_AIR,
    BT_GRASS, // TODO: Kill this.
    BT_DIRT,
    BT_STONE
};


bool IsBlockTypeFilled(BlockType block_type);
bool IsBlockTypeEmpty(BlockType block_type);


enum BlockSurface
{
    SURF_NONE,
    SURF_GRASS,
    SURF_DIRT,
    SURF_STONE
};


class Block
{
public:
    Block();

    BlockType getBlockType() const { return m_block_type; }
    void setBlockType(BlockType val) { m_block_type = val; }

    void clearExposureFlags();

    bool getExposure(FaceEnum face) const;
    void setExposure(FaceEnum face, bool val);
    bool hasExposures() const;

private:
    BlockType m_block_type;

    bool m_west_exposure;
    bool m_east_exposure;
    bool m_south_exposure;
    bool m_north_exposure;
    bool m_top_exposure;
    bool m_bottom_exposure;
};

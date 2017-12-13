#pragma once

#include "stdafx.h"
#include "draw_state_pt.h"
#include "utils.h"


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
    Block() :
        m_content(BT_AIR),
        m_west_exposed(false),
        m_east_exposed(false),
        m_south_exposed(false),
        m_north_exposed(false),
        m_top_exposed(false),
        m_bottom_exposed(false) {}

    BlockType getBlockType() const { return m_content; }
    void setBlockType(BlockType val) { m_content = val; }

    void clearExposureFlags();

    bool getExposure(FaceEnum face) const;
    void setExposure(FaceEnum face, bool val);

private:
    // Disallow copying.
    Block(const Block &that) = delete;
    void operator=(const Block &that) = delete;

    BlockType m_content;
    bool m_west_exposed;
    bool m_east_exposed;
    bool m_south_exposed;
    bool m_north_exposed;
    bool m_top_exposed;
    bool m_bottom_exposed;
};

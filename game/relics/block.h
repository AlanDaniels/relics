#pragma once

#include "stdafx.h"
#include "draw_state_pt.h"
#include "utils.h"


#include "common_util.h"

class Chunk;


enum BlockType : unsigned char
{
    BT_AIR   = 0,
    BT_DIRT  = 1,
    BT_STONE = 2
};


bool IsBlockTypeFilled(BlockType block_type);
bool IsBlockTypeEmpty (BlockType block_type);



enum SurfaceType : unsigned char
{
    SURF_GRASS_TOP = 0,
    SURF_DIRT      = 1,
    SURF_STONE     = 2,

    SURF_MAX_COUNT = 3,
    SURF_NOTHING   = 255
};


SurfaceType CalcSurfaceType(BlockType block_type, FaceEnum face, BlockType other);


// The block data itself.
class Block
{
public:
    Block();

    BlockType getBlockType() const { return m_block_type; }
    void setBlockType(BlockType val) { m_block_type = val; }

    void clearSurfaces();

    SurfaceType getSurface(FaceEnum face) const;
    void setSurface(FaceEnum face, SurfaceType val);

private:
    BlockType m_block_type;

    SurfaceType m_west_surf;
    SurfaceType m_east_surf;
    SurfaceType m_south_surf;
    SurfaceType m_north_surf;
    SurfaceType m_top_surf;
    SurfaceType m_bottom_surf;
};

static_assert(sizeof(Block) == 7, "Make sure blocks are nice and small");

#pragma once

#include "stdafx.h"
#include "draw_state_pt.h"
#include "utils.h"


#include "common_util.h"

class Chunk;


enum class BlockType : unsigned char
{
    AIR   = 0,
    DIRT  = 1,
    STONE = 2,
    COAL  = 3
};


bool IsBlockTypeFilled(BlockType block_type);
bool IsBlockTypeEmpty (BlockType block_type);



enum class SurfaceType : unsigned char
{
    GRASS_TOP = 0,
    DIRT      = 1,
    STONE     = 2,
    COAL      = 3,

    NOTHING   = 255
};

const int SURFACE_TYPE_COUNT = 4;


SurfaceType CalcSurfaceType(BlockType block_type, FaceType face, BlockType other);


// The block data itself.
class Block
{
public:
    Block();

    BlockType getBlockType() const { return m_block_type; }
    void setBlockType(BlockType val) { m_block_type = val; }

    void clearSurfaces();

    SurfaceType getSurface(FaceType face) const;
    void setSurface(FaceType face, SurfaceType val);

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

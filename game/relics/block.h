#pragma once

#include "stdafx.h"
#include "draw_state_pt.h"
#include "utils.h"


#include "common_util.h"

class Chunk;


enum BlockType
{
    BT_AIR   = 0,
    BT_DIRT  = 1,
    BT_STONE = 2
};


bool IsBlockTypeFilled(BlockType block_type);
bool IsBlockTypeEmpty (BlockType block_type);



enum SurfaceType
{
    SURF_GRASS  = 0,
    SURF_DIRT   = 1,
    SURF_STONE  = 2,

    SURF_MAX_COUNT = 3,
    SURF_NOTHING   = 255
};


SurfaceType CalcSurfaceType(BlockType block_type, FaceEnum face, BlockType other);


// As we rebuild, keep track of how many surfaces there are.
class SurfaceTotals
{
public:
    SurfaceTotals();
    void reset();
    void increment(SurfaceType surf_type);
    void add(const SurfaceTotals &that);
    int  get(SurfaceType surf) const;
    bool hasAnything() const;

private:
    std::array<int, SURF_MAX_COUNT> m_counts;
};


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
    SurfaceTotals getSurfaceTotals() const;

private:
    BlockType m_block_type;

    SurfaceType m_west_surface;
    SurfaceType m_east_surface;
    SurfaceType m_south_surface;
    SurfaceType m_north_surface;
    SurfaceType m_top_surface;
    SurfaceType m_bottom_surface;
};

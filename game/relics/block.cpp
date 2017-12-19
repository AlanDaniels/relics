
#include "stdafx.h"
#include "block.h"

#include "common_util.h"
#include "config.h"
#include "utils.h"

// Return true if a block type should generate ever landscape surfaces.
bool IsBlockTypeFilled(BlockType block_type)
{
    return (block_type != BT_AIR);
}


// And the opposite.
bool IsBlockTypeEmpty(BlockType block_type)
{
    return (block_type == BT_AIR);
}


// What surface, if any, should go between two blocks?
SurfaceType CalcSurfaceType(BlockType block_type, FaceEnum face, BlockType other)
{
    bool draw_transitions = GetConfig().debug.draw_transitions;

    SurfaceType result = SURF_NOTHING;

    switch (block_type) {
    case BT_DIRT:  
        if (other == BT_AIR) { 
            result = SURF_DIRT;
        }
        break;

    case BT_STONE:
        if (other == BT_AIR) {
            result = SURF_STONE;
        }
        else if ((other == BT_DIRT) && draw_transitions) {
            result = SURF_STONE;
        }
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, block_type);
        break;
    }

    return result;
}


// Default ctor. This is a plain struct, so it's all we need.
Block::Block() :
    m_block_type(BT_AIR),
    m_west_surf(SURF_NOTHING),
    m_east_surf(SURF_NOTHING),
    m_south_surf(SURF_NOTHING),
    m_north_surf(SURF_NOTHING),
    m_top_surf(SURF_NOTHING),
    m_bottom_surf(SURF_NOTHING)
{
}


// Clear the exposure flags.
void Block::clearSurfaces()
{
    m_west_surf   = SURF_NOTHING;
    m_east_surf   = SURF_NOTHING;
    m_south_surf  = SURF_NOTHING;
    m_north_surf  = SURF_NOTHING;
    m_top_surf    = SURF_NOTHING;
    m_bottom_surf = SURF_NOTHING;
}


// See if a block face is exposed.
SurfaceType Block::getSurface(FaceEnum face) const
{
    switch (face) {
    case FACE_WEST:   return m_west_surf;
    case FACE_EAST:   return m_east_surf;
    case FACE_SOUTH:  return m_south_surf;
    case FACE_NORTH:  return m_north_surf;
    case FACE_TOP:    return m_top_surf;
    case FACE_BOTTOM: return m_bottom_surf;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return SURF_NOTHING;
    }
}


// Set if a face is exposed.
void Block::setSurface(FaceEnum face, SurfaceType val)
{
    switch (face) {
    case FACE_WEST:   m_west_surf   = val; break;
    case FACE_EAST:   m_east_surf   = val; break;
    case FACE_SOUTH:  m_south_surf  = val; break;
    case FACE_NORTH:  m_north_surf  = val; break;
    case FACE_TOP:    m_top_surf    = val; break;
    case FACE_BOTTOM: m_bottom_surf = val; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return;
    }
}

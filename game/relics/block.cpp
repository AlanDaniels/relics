
#include "stdafx.h"
#include "block.h"

#include "common_util.h"
#include "config.h"
#include "utils.h"

// Return true if a block type should generate ever landscape surfaces.
bool IsBlockTypeFilled(BlockType block_type)
{
    return (block_type != BlockType::AIR);
}


// And the opposite.
bool IsBlockTypeEmpty(BlockType block_type)
{
    return (block_type == BlockType::AIR);
}


// What surface, if any, should go between two blocks?
SurfaceType CalcSurfaceType(BlockType block_type, FaceType face, BlockType other)
{
    bool draw_transitions = GetConfig().debug.draw_transitions;

    SurfaceType result = SURF_NOTHING;

    switch (block_type) {
    case BlockType::DIRT:
        if (other == BlockType::AIR) {
            result = SURF_DIRT;
        }
        break;

    case BlockType::STONE:
        if (other == BlockType::AIR) {
            result = SURF_STONE;
        }
        else {
            bool show = (other == BlockType::DIRT);
            if (show && draw_transitions) {
                result = SURF_STONE;
            }
        }
        break;

    case BlockType::COAL:
        if (other == BlockType::AIR) {
            result = SURF_COAL;
        }
        else {
            bool show = ((other == BlockType::DIRT) || 
                         (other == BlockType::STONE));
            if (show && draw_transitions) {
                result = SURF_COAL;
            }
        }
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(block_type));
        break;
    }

    return result;
}


// Default ctor. This is a plain struct, so it's all we need.
Block::Block() :
    m_block_type(BlockType::AIR),
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
SurfaceType Block::getSurface(FaceType face) const
{
    switch (face) {
    case FaceType::WEST:   return m_west_surf;
    case FaceType::EAST:   return m_east_surf;
    case FaceType::SOUTH:  return m_south_surf;
    case FaceType::NORTH:  return m_north_surf;
    case FaceType::TOP:    return m_top_surf;
    case FaceType::BOTTOM: return m_bottom_surf;
    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(face));
        return SURF_NOTHING;
    }
}


// Set if a face is exposed.
void Block::setSurface(FaceType face, SurfaceType val)
{
    switch (face) {
    case FaceType::WEST:   m_west_surf   = val; break;
    case FaceType::EAST:   m_east_surf   = val; break;
    case FaceType::SOUTH:  m_south_surf  = val; break;
    case FaceType::NORTH:  m_north_surf  = val; break;
    case FaceType::TOP:    m_top_surf    = val; break;
    case FaceType::BOTTOM: m_bottom_surf = val; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(face));
        return;
    }
}

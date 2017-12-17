
#include "stdafx.h"
#include "block.h"
#include "common_util.h"

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

// Default ctor. This is a plain struct, so it's all we need.
Block::Block() :
    m_block_type(BT_AIR),
    m_west_exposure(false),
    m_east_exposure(false),
    m_south_exposure(false),
    m_north_exposure(false),
    m_top_exposure(false),
    m_bottom_exposure(false) 
{
}


// Clear the exposure flags.
void Block::clearExposureFlags()
{
    m_west_exposure   = false;
    m_east_exposure   = false;
    m_south_exposure  = false;
    m_north_exposure  = false;
    m_top_exposure    = false;
    m_bottom_exposure = false;
}


// See if a block face is exposed.
bool Block::getExposure(FaceEnum face) const
{
    switch (face) {
    case FACE_WEST:   return m_west_exposure;
    case FACE_EAST:   return m_east_exposure;
    case FACE_SOUTH:  return m_south_exposure;
    case FACE_NORTH:  return m_north_exposure;
    case FACE_TOP:    return m_top_exposure;
    case FACE_BOTTOM: return m_bottom_exposure;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return false;
    }
}


// Set if a face is exposed.
void Block::setExposure(FaceEnum face, bool val)
{
    switch (face) {
    case FACE_WEST:   m_west_exposure   = val; break;
    case FACE_EAST:   m_east_exposure   = val; break;
    case FACE_SOUTH:  m_south_exposure  = val; break;
    case FACE_NORTH:  m_north_exposure  = val; break;
    case FACE_TOP:    m_top_exposure    = val; break;
    case FACE_BOTTOM: m_bottom_exposure = val; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return;
    }
}


// Return true if this block has any exposed faces.
bool Block::hasExposures() const
{
    return (
        m_west_exposure  || m_east_exposure  ||
        m_south_exposure || m_north_exposure ||
        m_top_exposure   || m_bottom_exposure);
}
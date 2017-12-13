
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


// Clear the exposure flags.
void Block::clearExposureFlags()
{
    m_west_exposed   = false;
    m_east_exposed   = false;
    m_south_exposed  = false;
    m_north_exposed  = false;
    m_top_exposed    = false;
    m_bottom_exposed = false;
}


// See if a block face is exposed.
bool Block::getExposure(FaceEnum face) const
{
    switch (face) {
    case FACE_WEST:   return m_west_exposed;
    case FACE_EAST:   return m_east_exposed;
    case FACE_SOUTH:  return m_south_exposed;
    case FACE_NORTH:  return m_north_exposed;
    case FACE_TOP:    return m_top_exposed;
    case FACE_BOTTOM: return m_bottom_exposed;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return false;
    }
}


// Set if a face is exposed.
void Block::setExposure(FaceEnum face, bool val)
{
    switch (face) {
    case FACE_WEST:   m_west_exposed   = val; break;
    case FACE_EAST:   m_east_exposed   = val; break;
    case FACE_SOUTH:  m_south_exposed  = val; break;
    case FACE_NORTH:  m_north_exposed  = val; break;
    case FACE_TOP:    m_top_exposed    = val; break;
    case FACE_BOTTOM: m_bottom_exposed = val; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return;
    }
}


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


// What surface, if any, should go between two blocks?
SurfaceType CalcSurfaceType(BlockType block_type, FaceEnum face, BlockType other)
{
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
        break;

    default:
        PrintTheImpossible(__FILE__, __LINE__, block_type);
        break;
    }

    if (result != SURF_NOTHING) {
        printf("");
    }
    return result;
}


// Surface totals ctor.
SurfaceTotals::SurfaceTotals()
{
    for (int i = 0; i < SURF_MAX_COUNT; i++) {
        m_counts[i] = 0;
    }
}


// Surface totals ctor.
void SurfaceTotals::reset()
{
    for (int i = 0; i < SURF_MAX_COUNT; i++) {
        m_counts[i] = 0;
    }
}


// Increment our surface type.
void SurfaceTotals::increment(SurfaceType surf)
{
    if (surf == SURF_NOTHING) {
        return;
    }

    assert(surf < SURF_MAX_COUNT);
    m_counts[surf]++;
}


// Add another.
void SurfaceTotals::add(const SurfaceTotals &that) 
{
    for (int i = 0; i < SURF_MAX_COUNT; i++) {
        m_counts[i] += that.m_counts[i];
    }
}


// Get the count.
int SurfaceTotals::get(SurfaceType surf) const 
{
    assert(surf < SURF_MAX_COUNT);
    return m_counts[surf];
}


// Do we have anything at all?
bool SurfaceTotals::hasAnything() const 
{
    for (int i = 0; i < SURF_MAX_COUNT; i++) {
        if (m_counts[i] > 0) {
            return true;
        }
    }

    return false;
}


// Default ctor. This is a plain struct, so it's all we need.
Block::Block() :
    m_block_type(BT_AIR),
    m_west_surface(SURF_NOTHING),
    m_east_surface(SURF_NOTHING),
    m_south_surface(SURF_NOTHING),
    m_north_surface(SURF_NOTHING),
    m_top_surface(SURF_NOTHING),
    m_bottom_surface(SURF_NOTHING)
{
}


// Clear the exposure flags.
void Block::clearSurfaces()
{
    m_west_surface   = SURF_NOTHING;
    m_east_surface   = SURF_NOTHING;
    m_south_surface  = SURF_NOTHING;
    m_north_surface  = SURF_NOTHING;
    m_top_surface    = SURF_NOTHING;
    m_bottom_surface = SURF_NOTHING;
}


// See if a block face is exposed.
SurfaceType Block::getSurface(FaceEnum face) const
{
    switch (face) {
    case FACE_WEST:   return m_west_surface;
    case FACE_EAST:   return m_east_surface;
    case FACE_SOUTH:  return m_south_surface;
    case FACE_NORTH:  return m_north_surface;
    case FACE_TOP:    return m_top_surface;
    case FACE_BOTTOM: return m_bottom_surface;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return SURF_NOTHING;
    }
}


// Set if a face is exposed.
void Block::setSurface(FaceEnum face, SurfaceType val)
{
    switch (face) {
    case FACE_WEST:   m_west_surface   = val; break;
    case FACE_EAST:   m_east_surface   = val; break;
    case FACE_SOUTH:  m_south_surface  = val; break;
    case FACE_NORTH:  m_north_surface  = val; break;
    case FACE_TOP:    m_top_surface    = val; break;
    case FACE_BOTTOM: m_bottom_surface = val; break;
    default:
        PrintTheImpossible(__FILE__, __LINE__, face);
        return;
    }
}


// Return true if this block has any exposed faces.
SurfaceTotals Block::getSurfaceTotals() const
{
    SurfaceTotals result;
    result.increment(m_west_surface);
    result.increment(m_east_surface);
    result.increment(m_south_surface);
    result.increment(m_north_surface);
    result.increment(m_top_surface);
    result.increment(m_bottom_surface);
    return result;
}
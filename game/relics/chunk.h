#pragma once

#include "stdafx.h"
#include "block.h"
#include "chunk_stripe.h"
#include "draw_state_pnt.h"


class Chunk;
class GameWorld;


#if 0
    int getCount(BlockSurface surf) const;
    const VertList_PNT *get_RO(BlockSurface surf) const;
    VertList_PNT *get(BlockSurface surf);
#endif


// The starting X and Z grid coords for a chunk.
class ChunkOrigin
{
public:
    ChunkOrigin() :
        m_x(0),
        m_z(0),
        m_debug_x(0),
        m_debug_z(0) {}

    // Chunks always have to start at regular boundaries.
    ChunkOrigin(int x, int z) :
        m_x(x),
        m_z(z) {
        assert((x % CHUNK_WIDTH) == 0);
        assert((z % CHUNK_WIDTH) == 0);
        m_debug_x = x / CHUNK_WIDTH;
        m_debug_z = z / CHUNK_WIDTH;
    }

    ChunkOrigin(const ChunkOrigin &that) :
        m_x(that.m_x),
        m_z(that.m_z),
        m_debug_x(that.m_debug_x),
        m_debug_z(that.m_debug_z) {}

    ChunkOrigin &operator=(const ChunkOrigin &that) {
        m_x = that.m_x;
        m_z = that.m_z;
        m_debug_x = that.m_debug_x;
        m_debug_z = that.m_debug_z;
        return *this;
    }

    // Equals operator, for fast comparisons.
    bool operator==(const ChunkOrigin &that) const {
        return ((m_x == that.m_x) && 
                (m_z == that.m_z));
    }

    // And a not-equals operator. God, C++ is picky.
    bool operator!=(const ChunkOrigin &that) const {
        return ((m_x != that.m_x) || 
                (m_z != that.m_z));
    }

    // Less than operator, since we use this as a map key.
    bool operator<(const ChunkOrigin &that) const {
        if      (m_x < that.m_x) { return true;  }
        else if (m_x > that.m_x) { return false; }
        else if (m_z < that.m_z) { return true;  }
        else if (m_z > that.m_z) { return false; }
        else { return false; }
    }

    inline int x() const { return m_x; }
    inline int z() const { return m_z; }

private:
    // Temporary, for the sake of checking our math.
    int m_debug_x;
    int m_debug_z;

    // The real values.
    int m_x;
    int m_z;
};


ChunkOrigin WorldToChunkOrigin(const MyVec4 &pos);


class Chunk
{
public:
    Chunk::Chunk(const GameWorld &world, const ChunkOrigin &origin);
    Chunk::~Chunk() {}

    bool IsGlobalGridWithin(const GlobalGrid &coord) const;

    BlockType getBlockType(const LocalGrid &coord) const;
    void setBlockType(const LocalGrid &coord, BlockType block_type);

    MyVec4 localGridToWorldPos(int local_x, int local_y, int local_z) const;

    int  getCountForSurface(SurfaceType surf) const;
    const VertList_PNT *getSurfaceList_RO(SurfaceType surf) const;
    VertList_PNT *getSurfaceList(SurfaceType surf);

    void recalcExposures();
    void realizeSurfaceLists();
    void unrealizeSurfaceLists();

    bool isAbovePlane(const MyPlane &plane) const;

    const ChunkOrigin getOrigin() const { return m_origin; }

    bool isUpToDate() const { return m_up_to_date; }
    bool isLandcsapeRealized() const { return m_realized; }

    const Chunk *getNeighborNorth() const;
    const Chunk *getNeighborSouth() const;
    const Chunk *getNeighborEast()  const;
    const Chunk *getNeighborWest()  const;

    std::string toDescription() const;

private:
    // Disallow default ctor, moving, and copying.
    Chunk() = delete;
    Chunk(const Chunk &that) = delete;
    void operator=(const Chunk &that) = delete;
    Chunk(Chunk &&that) = delete;
    void operator=(Chunk &&that) = delete;

    // Private methods.
    int getArrayOffset(int x, int y) const { return x + (CHUNK_WIDTH * y); }
    const ChunkStripe &getStripe_RO(int local_x, int local_y) const;
    ChunkStripe &getStripe(int local_x, int local_y);

    // Private data.
    const GameWorld &m_world;
    ChunkOrigin m_origin;

    bool m_up_to_date;
    bool m_realized;
    std::unique_ptr<VertList_PNT> m_grass_list;
    std::unique_ptr<VertList_PNT> m_dirt_list;
    std::unique_ptr<VertList_PNT> m_stone_list;

    std::array<ChunkStripe, CHUNK_WIDTH * CHUNK_HEIGHT> m_stripes;
};

#pragma once

#include "stdafx.h"
#include "block.h"
#include "chunk_stripe.h"
#include "draw_state_pnt.h"


class Chunk;
class GameWorld;


// The landscape vert lists for a chunk. We tried this as a map of 
// surface-enum-to-list, but the peformance was horrible, and there
// aren't that many lists anyway, so I'm hard-coding them here.
class ChunkVertLists {
public:
    ChunkVertLists() :
        m_grass_list(nullptr),
        m_dirt_list(nullptr),
        m_stone_list(nullptr),
        m_realized(false) {}

    ~ChunkVertLists();

    int getCount(BlockSurface surf) const;
    const VertList_PNT *get_RO(BlockSurface surf) const;
    VertList_PNT *get(BlockSurface surf);

    void resetLists();
    void realizeLists();
    bool areListsRealized() const { return m_realized; }

private:
    // Forbid copies.
    ChunkVertLists(const ChunkVertLists &that) = delete;
    void operator=(const ChunkVertLists &that) = delete;

    VertList_PNT *m_grass_list;
    VertList_PNT *m_dirt_list;
    VertList_PNT *m_stone_list;

    bool m_realized;
};


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
    Chunk(const GameWorld *pWorld, const ChunkOrigin &origin);

    bool IsGlobalGridWithin(const GlobalGrid &coord) const;

    const Block *getBlock_RO(const LocalGrid &coord) const;
    Block *getBlock(const LocalGrid &coord);

    MyVec4 localGridToWorldPos(int local_x, int local_y, int local_z) const;

    void recalcExposures();
    void realizeLandscape();

    bool isAbovePlane(const MyPlane &plane) const;

    const ChunkOrigin getOrigin() const { return m_origin; }
    const ChunkVertLists &getVertLists() const { return m_vert_lists; }
    bool areExposuresCurrent()  const { return m_exposures_are_current; }
    bool isLandcsapeRealized() const { return m_vert_lists.areListsRealized(); }

    const Chunk *getNeighborNorth() const;
    const Chunk *getNeighborSouth() const;
    const Chunk *getNeighborEast()  const;
    const Chunk *getNeighborWest()  const;

    std::string toDescription() const;

private:
    // Disallow copying.
    Chunk(const Chunk &that) = delete;
    void operator=(const Chunk &that) = delete;

    const ChunkStripe *getStripe_RO(int local_x, int local_y) const;
    ChunkStripe *getStripe(int local_x, int local_y);

    const GameWorld *m_world;
    ChunkOrigin m_origin;

    bool m_exposures_are_current;
    ChunkStripe *m_stripes[CHUNK_WIDTH][CHUNK_HEIGHT];
    ChunkVertLists m_vert_lists;
};

#pragma once

#include "stdafx.h"
#include "block.h"
#include "chunk_stripe.h"
#include "draw_state_pnt.h"


class Chunk;
class GameWorld;


// The landscape vert lists for a chunk.
// We tried this as a map of surface-to-list, but the peformance was horrible.
class LandscapeVertLists {
public:
    LandscapeVertLists() {}
    ~LandscapeVertLists();

    int getCount(BlockSurface surf) const;
    const VertList_PNT *get_RO(BlockSurface surf) const;
    VertList_PNT *get_RW(BlockSurface surf);
    void clear();
    void update();

private:
    // Forbid copies.
    LandscapeVertLists(const LandscapeVertLists &that) = delete;
    void operator=(const LandscapeVertLists &that) = delete;

    VertList_PNT *m_grass_list;
    VertList_PNT *m_dirt_list;
    VertList_PNT *m_stone_list;
    VertList_PNT *m_bedrock_list;
};


// The starting X and Z grid coords for a chunk.
class MyChunkOrigin
{
public:
    MyChunkOrigin() :
        m_x(0), m_z(0) {}

    // Chunks always have to start at regular boundaries.
    MyChunkOrigin(int x, int z) :
        m_x(x), m_z(z) {
        assert((x % CHUNK_WIDTH_X) == 0);
        assert((z % CHUNK_DEPTH_Z) == 0);
    }

    MyChunkOrigin(const MyChunkOrigin &that) :
        m_x(that.m_x),
        m_z(that.m_z) {}

    inline MyChunkOrigin &operator=(const MyChunkOrigin &that) {
        m_x = that.m_x;
        m_z = that.m_z;
        return *this;
    }

    // Less than operator, since we use this as a map key.
    inline bool operator<(const MyChunkOrigin &that) const {
        if      (m_x < that.m_x) { return true;  }
        else if (m_x > that.m_x) { return false; }
        else if (m_z < that.m_z) { return true;  }
        else if (m_z > that.m_z) { return false; }
        else { return false; }
    }

    inline int x() const { return m_x; }
    inline int z() const { return m_z; }

private:
    int m_x;
    int m_z;
};


MyChunkOrigin WorldToChunkOrigin(const MyVec4 &pos);


enum ChunkLoadStatus
{
    LOAD_STATUS_NONE,
    LOAD_STATUS_INTERIOR,
    LOAD_STATUS_COMPLETE
};


// TODO: We need to clean up the whole global vs. local coords thing.
class Chunk
{
public:
    Chunk(const GameWorld *pWorld, const MyChunkOrigin &origin);

    bool isCoordWithin(const MyGridCoord &coord) const;

    const Block *getBlockGlobal_RO(const MyGridCoord &coord) const;
    Block *getBlockGlobal_RW(const MyGridCoord &coord);

    const Block *getBlockLocal_RO(int local_x, int local_y, int local_z) const;
    Block *getBlockLocal_RW(int local_x, int local_y, int local_z);

    MyVec4 localToWorldCoord(int local_x, int local_y, int local_z) const;
    MyGridCoord globalToLocalCoord(const MyGridCoord &global_coord) const;

    ChunkLoadStatus getLoadStatus() const { return m_load_status; }

    void recalcInterior();
    void recalcEdges();
    void recalcAll();

    bool isAbovePlane(const MyPlane &plane) const;

    const MyChunkOrigin getOrigin() const { return m_origin; }

    const LandscapeVertLists &getLandscapeVertLists() const { return m_landscape_vert_lists; }

    const Chunk *getNeighborNorth() const;
    const Chunk *getNeighborSouth() const;
    const Chunk *getNeighborEast()  const;
    const Chunk *getNeighborWest()  const;

    std::string getDescription() const;

private:
    // Disallow copying.
    Chunk(const Chunk &that) = delete;
    void operator=(const Chunk &that) = delete;

    const ChunkStripe *getStripeLocal_RO(int local_x, int local_y) const;
    ChunkStripe *getStripeLocal_RW(int local_x, int local_y);

    const GameWorld *m_pWorld;
    MyChunkOrigin m_origin;

    ChunkLoadStatus m_load_status;
    ChunkStripe *m_pStripes[CHUNK_WIDTH_X][CHUNK_HEIGHT_Y];
    LandscapeVertLists m_landscape_vert_lists;
};

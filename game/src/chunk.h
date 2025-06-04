#pragma once

#include "stdafx.h"

#include "block.h"
#include "chunk_stripe.h"
#include "format.h"
#include "landscape.h"
#include "wavefront_object.h"


class Chunk;
class GameWorld;


// The starting X and Z grid coords for a chunk.
class ChunkOrigin
{
public:
    ChunkOrigin() :
        m_debug_x(0),
        m_debug_z(0),
        m_x(0),
        m_z(0) {}

    // Chunks always have to start at regular boundaries.
    ChunkOrigin(int x, int z) :
        m_debug_x(x / CHUNK_WIDTH),
        m_debug_z(z / CHUNK_WIDTH),
        m_x(x),
        m_z(z) {
        assert((x % CHUNK_WIDTH) == 0);
        assert((z % CHUNK_WIDTH) == 0);
    }

    ChunkOrigin(const ChunkOrigin &that) :
        m_debug_x(that.m_debug_x),
        m_debug_z(that.m_debug_z),
        m_x(that.m_x),
        m_z(that.m_z) {}

    ChunkOrigin &operator=(const ChunkOrigin &that) {
        m_debug_x = that.m_debug_x;
        m_debug_z = that.m_debug_z;
        m_x = that.m_x;
        m_z = that.m_z;
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

    // Convert this as a string.
    std::string toDescr() const {
        return fmt::format("Origin [{0}, {1}]", m_debug_x, m_debug_z);
    }

    inline int debugX() const { return m_debug_x; }
    inline int debugZ() const { return m_debug_z; }
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


// The chunk itself.
class Chunk
{
public:
    Chunk(const GameWorld &world, const ChunkOrigin &origin) :
        landscape(*this),
        m_world(world),
        m_origin(origin),
        m_last_touched_msecs(0) {}

    ~Chunk() {}

    // TODO: Hi Self! Happy day after Easter! Add these next.
    void joinMainThread();
    void departMainThread();

    void touch(int msecs) { m_last_touched_msecs = msecs; }
    int  getLastTouchedMsecs() const { return m_last_touched_msecs; }

    BlockType getBlockType(const LocalGrid &coord) const;
    void setBlockType(const LocalGrid &coord, BlockType block_type);

    bool   IsGlobalGridWithin(const GlobalGrid &coord) const;
    MyVec4 localGridToWorldPos(int local_x, int local_y, int local_z) const;

    bool isAbovePlane(const MyPlane &plane) const;
    const ChunkOrigin &getOrigin() const { return m_origin; }

    const Chunk *getNeighborNorth() const;
    const Chunk *getNeighborSouth() const;
    const Chunk *getNeighborEast()  const;
    const Chunk *getNeighborWest()  const;

    void addWFInstance(std::unique_ptr<WFInstance> obj) {
        m_wfinstance_list.emplace_back(std::move(obj));
    }

    // These definitions are getting ridiculous.
    const std::vector<std::unique_ptr<WFInstance>> &getWFInstances() const {
        return m_wfinstance_list;
    }

    std::string toString() const;

    // Methods needed by the specialty objects.
    void rebuildExposedBlockSet(SurfaceTotals *pOutTotals);
    void rebuildLandscape();

    std::vector<LocalGrid> getExposedBlockList();
    void addToSurfaceLists(const LocalGrid &coord);

    // Specialty objects.
    Landscape landscape;

private:
    FORBID_DEFAULT_CTOR(Chunk)
    FORBID_COPYING(Chunk)
    FORBID_MOVING(Chunk)

    // Private methods.
    int offset(int x, int y) const {
        return x + (CHUNK_WIDTH * y); 
    }

    // Private data.
    const GameWorld &m_world;
    ChunkOrigin m_origin;
    int m_last_touched_msecs;

    std::array<ChunkStripe, CHUNK_WIDTH * CHUNK_HEIGHT> m_stripes;

    std::set<LocalGrid> m_exposed_block_set;

    std::vector<std::unique_ptr<WFInstance>> m_wfinstance_list;
};

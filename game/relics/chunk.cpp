
#include "stdafx.h"
#include "chunk.h"
#include "common_util.h"

#include "block.h"
#include "chunk_stripe.h"
#include "draw_state_pnt.h"
#include "game_world.h"
#include "utils.h"


// Chunk Vert Lists destructor.
ChunkVertLists::~ChunkVertLists()
{
    if (m_grass_list != nullptr) {
        delete m_grass_list;
        m_grass_list = nullptr;
    }

    if (m_dirt_list != nullptr) {
        delete m_dirt_list;
        m_dirt_list = nullptr;
    }

    if (m_stone_list != nullptr) {
        delete m_stone_list;
        m_stone_list = nullptr;
    }

    if (m_bedrock_list != nullptr) {
        delete m_bedrock_list;
        m_bedrock_list = nullptr;
    }

    m_realized = false;
}


// Return the count for a particular surface type.
int ChunkVertLists::getCount(BlockSurface surf) const 
{
    switch (surf) {
    case SURF_GRASS:   return (m_grass_list   == nullptr) ? 0 : m_grass_list->getByteCount();
    case SURF_DIRT:    return (m_dirt_list    == nullptr) ? 0 : m_dirt_list->getByteCount();
    case SURF_STONE:   return (m_stone_list   == nullptr) ? 0 : m_stone_list->getByteCount();
    case SURF_BEDROCK: return (m_bedrock_list == nullptr) ? 0 : m_bedrock_list->getByteCount();
    default:
        PrintTheImpossible(__FILE__, __LINE__, surf);
        return 0;
    }
}


// Return a read-only version of the list for a particular surface.
// It's okay for a list to be null if it hasn't been used yet.
const VertList_PNT *ChunkVertLists::get_RO(BlockSurface surf) const 
{
    switch (surf) {
    case SURF_GRASS:   return m_grass_list;
    case SURF_DIRT:    return m_dirt_list;
    case SURF_STONE:   return m_stone_list;
    case SURF_BEDROCK: return m_bedrock_list;
    default:
        PrintTheImpossible(__FILE__, __LINE__, surf);
        return nullptr;
    }
}


// Return a verison of the list, for writing.
// If the list hasn't been created, do that now.
VertList_PNT *ChunkVertLists::get(BlockSurface surf) 
{
    switch (surf) {
    case SURF_GRASS:
        if (m_grass_list == nullptr) {
            m_grass_list = new VertList_PNT;
        }
        return m_grass_list;

    case SURF_DIRT:
        if (m_dirt_list == nullptr) {
            m_dirt_list = new VertList_PNT;
        }
        return m_dirt_list;

    case SURF_STONE:
        if (m_stone_list == nullptr) {
            m_stone_list = new VertList_PNT;
        }
        return m_stone_list;

    case SURF_BEDROCK:
        if (m_bedrock_list == nullptr) {
            m_bedrock_list = new VertList_PNT;
        }
        return m_bedrock_list;

    default:
        PrintTheImpossible(__FILE__, __LINE__, surf);
        return nullptr;
    }
}


// Reset all out our lists.
void ChunkVertLists::resetLists() 
{
    if (m_grass_list != nullptr) { 
        m_grass_list->reset(); 
    }
    if (m_dirt_list != nullptr) {
        m_dirt_list->reset();
    }
    if (m_stone_list != nullptr) {
        m_stone_list->reset();
    }
    if (m_bedrock_list != nullptr) {
        m_bedrock_list->reset();
    }

    m_realized = false;
}


// Realize any lists we've created so far.
void ChunkVertLists::realizeLists() 
{
    if (m_grass_list != nullptr) {
        m_grass_list->realize();
    }

    if (m_dirt_list != nullptr) {
        m_dirt_list->realize();
    }

    if (m_stone_list != nullptr) {
        m_stone_list->realize();
    }

    if (m_bedrock_list != nullptr) {
        m_bedrock_list->realize();
    }

    m_realized = true;
}


// Turn a world coord into a chunk origin.
ChunkOrigin WorldToChunkOrigin(const MyVec4 &pos)
{
    int grid_x = static_cast<int>(floor(pos.x() / BLOCK_SCALE));
    int grid_z = static_cast<int>(floor(pos.z() / BLOCK_SCALE));

    int x = RoundDownInt(grid_x, CHUNK_WIDTH);
    int z = RoundDownInt(grid_z, CHUNK_WIDTH);

    return ChunkOrigin(x, z);
}


// Chunk values ctor.
// We have to use an array of pointers here, since we want each stripe to be aware
// of its owner and position. So, we can't use references since that would get all circular.
// Once that's done, feel free to use "getStripe" everywhere else.
Chunk::Chunk(const GameWorld *world, const ChunkOrigin &origin) :
    m_world(world),
    m_origin(origin),
    m_exposures_are_current(false)
{
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            m_stripes[x][y] = new ChunkStripe(this, x, y);
        }
    }
}


// Get a chunk stripe, read-only version.
const ChunkStripe *Chunk::getStripe_RO(int local_x, int local_y) const
{
    assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
    assert((local_y >= 0) && (local_y < CHUNK_HEIGHT));

    return m_stripes[local_x][local_y];
}


// Gert a chunk stripe, writeable version.
ChunkStripe *Chunk::getStripe(int local_x, int local_y)
{
    assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
    assert((local_y >= 0) && (local_y < CHUNK_HEIGHT));

    return m_stripes[local_x][local_y];
}


// Return true if a grid coord is within this chunk.
bool Chunk::isCoordWithin(const GlobalGrid &coord) const
{
    int local_x = coord.x() - m_origin.x();
    int local_y = coord.y();
    int local_z = coord.z() - m_origin.z();

    bool result = (
        (local_x >= 0) && (local_x < CHUNK_WIDTH)  &&
        (local_y >= 0) && (local_y < CHUNK_HEIGHT) &&
        (local_z >= 0) && (local_z < CHUNK_WIDTH));
    return result;
}


// Get the block at a particular grid coord, read-only version.
const Block *Chunk::getBlockGlobal_RO(const GlobalGrid &coord) const
{
    assert(isCoordWithin(coord));

    int local_x = coord.x() - m_origin.x();
    int local_y = coord.y();
    int local_z = coord.z() - m_origin.z();

    return getStripe_RO(local_x, local_y)->getBlock_RO(local_z);
}


// Get the block at a particular grid coord, read-only version.
Block *Chunk::getBlockGlobal(const GlobalGrid &coord)
{
    assert(isCoordWithin(coord));

    int local_x = coord.x() - m_origin.x();
    int local_y = coord.y();
    int local_z = coord.z() - m_origin.z();

    return getStripe(local_x, local_y)->getBlock_RW(local_z);
}


// Get the block at a particular local coord, read-only version.
const Block *Chunk::getBlockLocal_RO(int local_x, int local_y, int local_z) const
{
    assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
    assert((local_y >= 0) && (local_y < CHUNK_HEIGHT));
    assert((local_z >= 0) && (local_z < CHUNK_WIDTH));

    return getStripe_RO(local_x, local_y)->getBlock_RO(local_z);
}


// Get the block at a particular local coord, writeable version.
Block *Chunk::getBlockLocal(int local_x, int local_y, int local_z)
{
    assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
    assert((local_y >= 0) && (local_y < CHUNK_HEIGHT));
    assert((local_z >= 0) && (local_z < CHUNK_WIDTH));

    return getStripe(local_x, local_y)->getBlock_RW(local_z);
}


// Recalc the entire chunk from scratch.
// Note that this is a separate step from realizing our vert lists!
void Chunk::recalcExposures()
{
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            getStripe(x, y)->recalcExposures();
        }
    }

    m_exposures_are_current = true;
}


//  Realize all our vertt lists.
void Chunk::realizeLandscape()
{
    // We must only call this when our landscape is current.
    assert(m_exposures_are_current);

    m_vert_lists.resetLists();

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            getStripe(x, y)->addToVertLists(&m_vert_lists);
        }
    }

    m_vert_lists.realizeLists();
}


// Return true if any of the eight corners of the chunk are "above" a plane.
bool Chunk::isAbovePlane(const MyPlane &plane) const
{
    GLfloat west   = GridToWorld(m_origin.x());
    GLfloat east   = GridToWorld(m_origin.x() + CHUNK_WIDTH);
    GLfloat south  = GridToWorld(m_origin.z());
    GLfloat north  = GridToWorld(m_origin.z() + CHUNK_WIDTH);
    GLfloat top    = GridToWorld(CHUNK_HEIGHT);
    GLfloat bottom = GridToWorld(0);

    MyVec4 top_SW(west, top, south);
    MyVec4 top_SE(east, top, south);
    MyVec4 top_NW(west, top, north);
    MyVec4 top_NE(east, top, north);

    MyVec4 bottom_SW(west, bottom, south);
    MyVec4 bottom_SE(east, bottom, south);
    MyVec4 bottom_NW(west, bottom, north);
    MyVec4 bottom_NE(east, bottom, north);

    bool result = (
        (plane.distanceToPoint(top_SW) >= EPSILON) ||
        (plane.distanceToPoint(top_SE) >= EPSILON) ||
        (plane.distanceToPoint(top_NW) >= EPSILON) ||
        (plane.distanceToPoint(top_NE) >= EPSILON) ||
        (plane.distanceToPoint(bottom_SW) >= EPSILON) ||
        (plane.distanceToPoint(bottom_SE) >= EPSILON) ||
        (plane.distanceToPoint(bottom_NW) >= EPSILON) ||
        (plane.distanceToPoint(bottom_NE) >= EPSILON));

    return result;
}


// Convert a local coord to a graphics coord.
MyVec4 Chunk::localToWorldCoord(int x, int y, int z) const
{
    // Note the "less than or equal" to allow for edge cases.
    assert((x >= 0) && (x <= CHUNK_WIDTH));
    assert((y >= 0) && (y <= CHUNK_HEIGHT));
    assert((z >= 0) && (z <= CHUNK_WIDTH));

    int global_x = m_origin.x() + x;
    int global_y = y;
    int global_z = m_origin.z() + z;

    return MyVec4(GridToWorld(global_x), GridToWorld(global_y), GridToWorld(global_z));
}


// Get our neigbor to the north (positive Z).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborNorth() const
{
    int x = m_origin.x();
    int z = m_origin.z() + CHUNK_WIDTH;
    return m_world->getOptionalChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the south (negative Z).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborSouth() const
{
    int x = m_origin.x();
    int z = m_origin.z() - CHUNK_WIDTH;
    return m_world->getOptionalChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the east (positive X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborEast() const
{
    int x = m_origin.x() + CHUNK_WIDTH;
    int z = m_origin.z();
    return m_world->getOptionalChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the west (negative X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborWest() const
{
    int x = m_origin.x() - CHUNK_WIDTH;
    int z = m_origin.z();
    return m_world->getOptionalChunk(ChunkOrigin(x, z));
}


// For debugging, print out all the details about this chunk.
std::string Chunk::toDescription() const
{
    std::string result;

    // Count our block types.
    int dirt_blocks  = 0;
    int stone_blocks = 0;

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_WIDTH; z++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                const Block *pBlock = getBlockLocal_RO(x, y, z);
                switch (pBlock->getContent()) {
                case CONTENT_DIRT:  dirt_blocks++; break;
                case CONTENT_STONE: stone_blocks++; break;
                default: break;
                }
            }
        }
    }

    // Count our exposures.
    int grass_surfaces = m_vert_lists.getCount(SURF_GRASS);
    int dirt_surfaces  = m_vert_lists.getCount(SURF_DIRT);
    int stone_surfaces = m_vert_lists.getCount(SURF_STONE);

    const char *exposures_true = areExposuresCurrent()  ? "true" : "false";
    const char *realized_true  = isLandcsapeRealized() ? "true" : "false";

    char msg[64];

    sprintf(msg, "Chunk at [%d, %d]\n", m_origin.x(), m_origin.z());
    result += msg;
    sprintf(msg, "  Exposures: %s, Realized: %s\n", exposures_true, realized_true);
    result += msg;
    sprintf(msg, "  Block = dirt: %d\n",  dirt_blocks);
    result += msg;
    sprintf(msg, "  Block = stone: %d\n", stone_blocks);
    result += msg;
    sprintf(msg, "  Surface = grass: %d\n", grass_surfaces);
    result += msg;
    sprintf(msg, "  Surface = dirt:  %d\n", dirt_surfaces);
    result += msg;
    sprintf(msg, "  Surface = stone: %d\n", stone_surfaces);
    result += msg;

    return result;
}

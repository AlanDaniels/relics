
#include "stdafx.h"
#include "chunk.h"
#include "common_util.h"

#include "block.h"
#include "chunk_stripe.h"
#include "draw_state_pnt.h"
#include "game_world.h"
#include "utils.h"


// Return a read-only version of the list for a particular surface.
// It's okay for a list to be null if it hasn't been used yet.
const VertList_PNT *Chunk::getSurfaceList_RO(SurfaceType surf) const 
{
    switch (surf) {
    case SURF_GRASS: return (m_grass_list != nullptr) ? m_grass_list.get() : nullptr;
    case SURF_DIRT:  return (m_dirt_list  != nullptr) ? m_dirt_list.get()  : nullptr;
    case SURF_STONE: return (m_stone_list != nullptr) ? m_stone_list.get() : nullptr;
    default:
        PrintTheImpossible(__FILE__, __LINE__, surf);
        return nullptr;
    }
}


// Realize any lists we've created so far.
void Chunk::realizeSurfaceLists() 
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

    m_realized = true;
}


// Un-realize any lists we've created so far.
void Chunk::unrealizeSurfaceLists()
{
    if (m_grass_list != nullptr) {
        m_grass_list->unrealize();
    }

    if (m_dirt_list != nullptr) {
        m_dirt_list->unrealize();
    }

    if (m_stone_list != nullptr) {
        m_stone_list->unrealize();
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


// Ctor from values.
Chunk::Chunk(const GameWorld &world, const ChunkOrigin &origin) :
    m_world(world),
    m_origin(origin),
    m_up_to_date(false),
    m_realized(false),
    m_grass_list(nullptr),
    m_dirt_list(nullptr),
    m_stone_list(nullptr)
{
}




// Get a chunk stripe, read-only version.
const ChunkStripe &Chunk::getStripe_RO(int local_x, int local_y) const
{
    assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
    assert((local_y >= 0) && (local_y < CHUNK_HEIGHT));

    int offset = getArrayOffset(local_x, local_y);
    return m_stripes[offset];
}


// Gert a chunk stripe, writeable version.
ChunkStripe &Chunk::getStripe(int local_x, int local_y)
{
    assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
    assert((local_y >= 0) && (local_y < CHUNK_HEIGHT));
    int offset = getArrayOffset(local_x, local_y);
    return m_stripes[offset];
}


// Return a verison of the list, for writing.
// If the list hasn't been created, do that now.
VertList_PNT *Chunk::getSurfaceList(SurfaceType surf)
{
    int FIGURE_OUT_VALUE = 100;

    switch (surf) {
    case SURF_GRASS:
        if (m_grass_list == nullptr) {
            m_grass_list = std::make_unique<VertList_PNT>(FIGURE_OUT_VALUE);
        }
        return m_grass_list.get();

    case SURF_DIRT:
        if (m_dirt_list == nullptr) {
            m_dirt_list = std::make_unique<VertList_PNT>(FIGURE_OUT_VALUE);
        }
        return m_dirt_list.get();

    case SURF_STONE:
        if (m_stone_list == nullptr) {
            m_stone_list = std::make_unique<VertList_PNT>(FIGURE_OUT_VALUE);
        }
        return m_stone_list.get();

    default:
        PrintTheImpossible(__FILE__, __LINE__, surf);
        return nullptr;
    }
}


// Return true if a grid coord is within this chunk.
bool Chunk::IsGlobalGridWithin(const GlobalGrid &coord) const
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


// Get the type of a block.
BlockType Chunk::getBlockType(const LocalGrid &coord) const
{
    const ChunkStripe &stripe = getStripe_RO(coord.x(), coord.y());
    BlockType result = stripe.getBlockType(coord.z());
    return result;
}


// Set the type of a block.

void Chunk::setBlockType(const LocalGrid &coord, BlockType block_type)
{
    ChunkStripe &stripe = getStripe(coord.x(), coord.y());
    stripe.setBlockType(coord.z(), block_type);
}


// Return the count for a particular surface type.
int Chunk::getCountForSurface(SurfaceType surf) const
{
    switch (surf) {
    case SURF_GRASS: return (m_grass_list == nullptr) ? 0 : m_grass_list->getByteCount();
    case SURF_DIRT:  return (m_dirt_list == nullptr) ? 0 : m_dirt_list->getByteCount();
    case SURF_STONE: return (m_stone_list == nullptr) ? 0 : m_stone_list->getByteCount();
    default:
        PrintTheImpossible(__FILE__, __LINE__, surf);
        return 0;
    }
}


// Recalc the entire chunk from scratch.
// Note that this is a separate step from realizing our vert lists!
void Chunk::recalcExposures()
{
    // Clear our any existing lists.
    if (m_grass_list != nullptr) {
        m_grass_list->reset();
    }
    if (m_dirt_list != nullptr) {
        m_dirt_list->reset();
    }
    if (m_stone_list != nullptr) {
        m_stone_list->reset();
    }

    SurfaceTotals totals;
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            totals.add(getStripe(x, y).recalcExposures(*this, x, y));
        }
    }

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            getStripe(x, y).addToSurfaceLists(*this, x, y);
        }
    }

    m_up_to_date = true;
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
MyVec4 Chunk::localGridToWorldPos(int x, int y, int z) const
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
    return m_world.getOptionalChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the south (negative Z).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborSouth() const
{
    int x = m_origin.x();
    int z = m_origin.z() - CHUNK_WIDTH;
    return m_world.getOptionalChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the east (positive X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborEast() const
{
    int x = m_origin.x() + CHUNK_WIDTH;
    int z = m_origin.z();
    return m_world.getOptionalChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the west (negative X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborWest() const
{
    int x = m_origin.x() - CHUNK_WIDTH;
    int z = m_origin.z();
    return m_world.getOptionalChunk(ChunkOrigin(x, z));
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
                LocalGrid coord(x, y, z);
                BlockType block_type = getBlockType(coord);

                switch (block_type) {
                case BT_DIRT:  dirt_blocks++; break;
                case BT_STONE: stone_blocks++; break;
                default: break;
                }
            }
        }
    }

    // Count our exposures.
    int grass_surfaces = getCountForSurface(SURF_GRASS);
    int dirt_surfaces  = getCountForSurface(SURF_DIRT);
    int stone_surfaces = getCountForSurface(SURF_STONE);

    const char *exposures_str = isUpToDate()  ? "true" : "false";
    const char *realized_str  = isLandcsapeRealized()  ? "true" : "false";

    std::unique_ptr<char[]> buffer(new char[1024]);

    sprintf(buffer.get(), "Chunk at [%d, %d]\n", m_origin.x(), m_origin.z());
    result += buffer.get();
    sprintf(buffer.get(), "  Exposures: %s, Realized: %s\n", exposures_str, realized_str);
    result += buffer.get();
    sprintf(buffer.get(), "  Block = dirt: %d\n",  dirt_blocks);
    result += buffer.get();
    sprintf(buffer.get(), "  Block = stone: %d\n", stone_blocks);
    result += buffer.get();
    sprintf(buffer.get(), "  Surface = grass: %d\n", grass_surfaces);
    result += buffer.get();
    sprintf(buffer.get(), "  Surface = dirt:  %d\n", dirt_surfaces);
    result += buffer.get();
    sprintf(buffer.get(), "  Surface = stone: %d\n", stone_surfaces);
    result += buffer.get();

    return result;
}

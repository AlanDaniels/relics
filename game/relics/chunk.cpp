
#include "stdafx.h"
#include "chunk.h"
#include "common_util.h"

#include "block.h"
#include "chunk_stripe.h"
#include "draw_state_pnt.h"
#include "format.h"
#include "game_world.h"
#include "utils.h"


// Turn a world coord into a chunk origin.
ChunkOrigin WorldToChunkOrigin(const MyVec4 &pos)
{
    int grid_x = static_cast<int>(floor(pos.x() / BLOCK_SCALE));
    int grid_z = static_cast<int>(floor(pos.z() / BLOCK_SCALE));

    int x = RoundDownInt(grid_x, CHUNK_WIDTH);
    int z = RoundDownInt(grid_z, CHUNK_WIDTH);

    return ChunkOrigin(x, z);
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
    int index = offset(coord.x(), coord.y());
    const ChunkStripe &stripe = m_stripes.at(index);
    BlockType result = stripe.getBlockType(coord.z());
    return result;
}


// Set the type of a block.
void Chunk::setBlockType(const LocalGrid &coord, BlockType block_type)
{
    int index = offset(coord.x(), coord.y());
    ChunkStripe &stripe = m_stripes.at(index);
    stripe.setBlockType(coord.z(), block_type);
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
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the south (negative Z).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborSouth() const
{
    int x = m_origin.x();
    int z = m_origin.z() - CHUNK_WIDTH;
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the east (positive X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborEast() const
{
    int x = m_origin.x() + CHUNK_WIDTH;
    int z = m_origin.z();
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the west (negative X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborWest() const
{
    int x = m_origin.x() - CHUNK_WIDTH;
    int z = m_origin.z();
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Rebuild our set of inner blocks that generate surfaces.
// We do this as a separate step since it doesn't involve other chunks.
void Chunk::rebuildInnerExposedBlockSet(SurfaceTotals *pOutTotals)
{
    m_inner_exposed_block_set.clear();

    for     (int x = 1; x < CHUNK_WIDTH - 1; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            int index = offset(x, y);
            for (int z = 1; z < CHUNK_WIDTH - 1; z++) {
                LocalGrid coord(x, y, z);
                bool exposed = m_stripes.at(index).recalcExposuresForBlock(*this, coord, pOutTotals);
                if (exposed) {
                    m_inner_exposed_block_set.insert(coord);
                }
            }
        }
    }

    m_status = ChunkStatus::INNER;
}


// Rebuild our set of blocks along our edge that generate surfaces.
// Note that *all* the blocks involves here involve other chunks.
void Chunk::rebuildEdgeExposedBlockSet(SurfaceTotals *pOutTotals)
{
    assert(m_status == ChunkStatus::INNER);

    m_edge_exposed_block_set.clear();

    // The western-most stripes.
    const int west_x = 0;
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        int index = offset(west_x, y);
        for (int z = 0; z < CHUNK_WIDTH; z++) {
            LocalGrid coord(west_x, y, z);
            bool exposed = m_stripes.at(index).recalcExposuresForBlock(*this, coord, pOutTotals);
            if (exposed) {
                m_edge_exposed_block_set.insert(coord);
            }
        }
    }

    // The middle stripes. Just check the southern-most and northern-most blocks.
    for (int x = 1; x < CHUNK_WIDTH - 1; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            int index = offset(x, y);

            LocalGrid south_coord(x, y, 0);
            bool south_exposed = m_stripes.at(index).recalcExposuresForBlock(*this, south_coord, pOutTotals);
            if (south_exposed) {
                m_edge_exposed_block_set.insert(south_coord);
            }

            LocalGrid north_coord(x, y, CHUNK_WIDTH - 1);
            bool north_exposed = m_stripes.at(index).recalcExposuresForBlock(*this, north_coord, pOutTotals);
            if (north_exposed) {
                m_edge_exposed_block_set.insert(north_coord);
            }
        }
    }

    // Finally, the eastern-most stripes.
    const int east_x = CHUNK_WIDTH - 1;
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        int index = offset(east_x, y);
        for (int z = 0; z < CHUNK_WIDTH; z++) {
            LocalGrid coord(east_x, y, z);
            bool exposed = m_stripes.at(index).recalcExposuresForBlock(*this, coord, pOutTotals);
            if (exposed) {
                m_edge_exposed_block_set.insert(coord);
            }
        }
    }

    m_status = ChunkStatus::EDGES;
}


// Get a vector of the exposed blocks.
std::vector<LocalGrid> Chunk::getExposedBlockList()
{
    int count = m_inner_exposed_block_set.size() + m_edge_exposed_block_set.size();

    std::vector<LocalGrid> results(count);
    results.insert(results.end(), m_inner_exposed_block_set.begin(), m_inner_exposed_block_set.end());
    results.insert(results.end(), m_edge_exposed_block_set.begin(),  m_edge_exposed_block_set.end());
    std::sort(results.begin(), results.end());
    
    return std::move(results);
}


// Add to surface lists.
void Chunk::addToSurfaceLists(const LocalGrid &coord)
{
    int index = offset(coord.x(), coord.y());
    m_stripes.at(index).addToSurfaceLists(*this, coord);
}


// For debugging, print out all the details about this chunk.
std::string Chunk::toString() const
{
    // Count our block types.
    int dirt_blocks  = 0;
    int stone_blocks = 0;
    int coal_blocks  = 0;

    for         (int x = 0; x < CHUNK_WIDTH;  x++) {
        for     (int z = 0; z < CHUNK_WIDTH;  z++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                LocalGrid coord(x, y, z);
                BlockType block_type = getBlockType(coord);

                switch (block_type) {
                case BlockType::DIRT:  dirt_blocks++;  break;
                case BlockType::STONE: stone_blocks++; break;
                case BlockType::COAL:  coal_blocks++;  break;
                default: break;
                }
            }
        }
    }

    // Count our surfaces.
    int grass_surfaces = landscape.getCountForSurface(SurfaceType::GRASS_TOP);
    int dirt_surfaces  = landscape.getCountForSurface(SurfaceType::DIRT);
    int stone_surfaces = landscape.getCountForSurface(SurfaceType::STONE);
    int coal_surfaces  = landscape.getCountForSurface(SurfaceType::COAL);

    // Print the results.
    std::string up_to_date = (getStatus() == ChunkStatus::LANDSCAPE) ? "true" : "false";

    std::string result =
        fmt::format("Chunk at [{0}, {1}]\n", m_origin.x(), m_origin.z()) +
        fmt::format("  Up To Date: {}\n", up_to_date) +
        fmt::format("  Block = dirt: {}\n",  dirt_blocks) +
        fmt::format("  Block = stone: {}\n", stone_blocks) +
        fmt::format("  Surface = grass: {}\n", grass_surfaces) +
        fmt::format("  Surface = dirt:  {}\n", dirt_surfaces) +
        fmt::format("  Surface = stone: {}\n", stone_surfaces);

    return result;
}

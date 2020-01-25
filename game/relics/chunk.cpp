
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
    const int grid_x = static_cast<int>(floor(pos.x() / BLOCK_SCALE));
    const int grid_z = static_cast<int>(floor(pos.z() / BLOCK_SCALE));

    const int x = RoundDownInt(grid_x, CHUNK_WIDTH);
    const int z = RoundDownInt(grid_z, CHUNK_WIDTH);

    return ChunkOrigin(x, z);
}


// Return true if a grid coord is within this chunk.
bool Chunk::IsGlobalGridWithin(const GlobalGrid &coord) const
{
    const int local_x = coord.x() - m_origin.x();
    const int local_y = coord.y();
    const int local_z = coord.z() - m_origin.z();

    const bool result = (
        (local_x >= 0) && (local_x < CHUNK_WIDTH)  &&
        (local_y >= 0) && (local_y < CHUNK_HEIGHT) &&
        (local_z >= 0) && (local_z < CHUNK_WIDTH));
    return result;
}


// Get the type of a block.
BlockType Chunk::getBlockType(const LocalGrid &coord) const
{
    const int index = offset(coord.x(), coord.y());
    const ChunkStripe &stripe = m_stripes.at(index);
    const BlockType result = stripe.getBlockType(coord.z());
    return result;
}


// Set the type of a block.
void Chunk::setBlockType(const LocalGrid &coord, BlockType block_type)
{
    const int index = offset(coord.x(), coord.y());
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

    const int global_x = m_origin.x() + x;
    const int global_y = y;
    const int global_z = m_origin.z() + z;

    return MyVec4(GridToWorld(global_x), GridToWorld(global_y), GridToWorld(global_z));
}


// Get our neigbor to the north (positive Z).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborNorth() const
{
    const int x = m_origin.x();
    const int z = m_origin.z() + CHUNK_WIDTH;
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the south (negative Z).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborSouth() const
{
    const int x = m_origin.x();
    const int z = m_origin.z() - CHUNK_WIDTH;
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the east (positive X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborEast() const
{
    const int x = m_origin.x() + CHUNK_WIDTH;
    const int z = m_origin.z();
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Get our neighbor to the west (negative X).
// This may not be loaded into the game world yet.
const Chunk *Chunk::getNeighborWest() const
{
    const int x = m_origin.x() - CHUNK_WIDTH;
    const int z = m_origin.z();
    return m_world.getChunk(ChunkOrigin(x, z));
}


// Rebuild our set of inner blocks that generate surfaces.
// We do this as a separate step since it doesn't involve other chunks.
// This resets our status back to the start.
void Chunk::rebuildExposedBlockSet(SurfaceTotals *pOutTotals)
{
    m_exposed_block_set.clear();

    for     (int x = 0; x < CHUNK_WIDTH;  x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            const int index = offset(x, y);
            for (int z = 0; z < CHUNK_WIDTH; z++) {
                const LocalGrid coord(x, y, z);
                const bool exposed = m_stripes.at(index).recalcExposuresForBlock(*this, coord, pOutTotals);
                if (exposed) {
                    m_exposed_block_set.insert(coord);
                }
            }
        }
    }
}


// Rebuild the lanscape vert lists. This should only be done here.
void Chunk::rebuildLandscape()
{
    landscape.rebuildSurfaceLists(); 
}


// Get a vector of the exposed blocks.
std::vector<LocalGrid> Chunk::getExposedBlockList()
{
    const int count = m_exposed_block_set.size();

    std::vector<LocalGrid> results(count);
    results.insert(results.end(), m_exposed_block_set.begin(), m_exposed_block_set.end());
    std::sort(results.begin(), results.end());
    
    return std::move(results);
}


// Add to surface lists.
void Chunk::addToSurfaceLists(const LocalGrid &coord)
{
    const int index = offset(coord.x(), coord.y());
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
                const LocalGrid coord(x, y, z);
                const BlockType block_type = getBlockType(coord);

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
    const int grass_surfaces = landscape.getCountForSurface(SurfaceType::GRASS_TOP);
    const int dirt_surfaces  = landscape.getCountForSurface(SurfaceType::DIRT);
    const int stone_surfaces = landscape.getCountForSurface(SurfaceType::STONE);
    const int coal_surfaces  = landscape.getCountForSurface(SurfaceType::COAL);

    // Print the results.
    std::string result =
        fmt::format("Chunk at [{0}, {1}]\n", m_origin.x(), m_origin.z()) +
        fmt::format("  Block = dirt: {}\n",  dirt_blocks) +
        fmt::format("  Block = stone: {}\n", stone_blocks) +
        fmt::format("  Surface = grass: {}\n", grass_surfaces) +
        fmt::format("  Surface = dirt:  {}\n", dirt_surfaces) +
        fmt::format("  Surface = stone: {}\n", stone_surfaces);

    return result;
}

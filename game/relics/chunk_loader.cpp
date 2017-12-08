
#include "stdafx.h"
#include "chunk_loader.h"

#include "block.h"
#include "chunk.h"
#include "simplex_noise.h"
#include "utils.h"


// Just throw a mutex around any time we're sampling the height image map.
std::atomic_bool s_initialized = false;
std::mutex s_mutex;
static sf::Image s_height_map_image;



// Init our chunk loader.
// Thread-safe via a mutex.
bool Init()
{
    std::lock_guard<std::mutex> guard(s_mutex);

    std::string fname = "map/map.png";
    if (!ReadImageResource(&s_height_map_image, fname)) {
        s_initialized = true;
        return false;
    }

    s_height_map_image.flipVertically();
    s_initialized = true;
    return true;
}


// Sample a point off the height map.
// Thread safe via a mutex.
int SampleHeightMap(int x, int y)
{
    std::lock_guard<std::mutex> guard(s_mutex);

    sf::Vector2u dims = s_height_map_image.getSize();
    int dim_x = dims.x;
    int dim_y = dims.y;

    int offset_x = (dim_x / 2) + x;
    int offset_y = (dim_y / 2) + y;

    // If we're completely off the image, don't bother.
    if ((offset_x < 0) || (offset_x >= dim_x) ||
        (offset_y < 0) || (offset_y >= dim_y)) {
        return 0;
    }

    sf::Color color = s_height_map_image.getPixel(offset_x, offset_y);

    // Meh, just go with intensity for now.
    int intensity = (color.r + color.g + color.b) / 3;

    // And, scale it down so we don't end up with monstrous mountains.
    intensity /= 8;
    return intensity;
}


// Load a chunk. This should ideally be called via "std::async".
// This just deals with the block data. The landscape is are dealt with later.
Chunk *LoadChunkAsync(const GameWorld *pWorld, const ChunkOrigin &origin)
{
    if (!s_initialized) {
        if (!Init()) {
            return nullptr;
        }
    }

    Chunk *result = new Chunk(pWorld, origin);

    // First, figure out the content type, from height-map sampling.
    for     (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_WIDTH; z++) {
            int dirt_level = SampleHeightMap(origin.x() + x, origin.z() + z);

            // TODO: Meh, flatten it for now.
            dirt_level = dirt_level / 2;

            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                BlockContent content;
                if (y == 0) {
                    content = CONTENT_BEDROCK;
                }
                else if (y < dirt_level) {
                    content = CONTENT_DIRT;
                }
                else if (y == dirt_level) {
                    content = CONTENT_GRASS;
                }
                else {
                    content = CONTENT_AIR;
                }

                result->getBlockLocal(x, y, z)->setContent(content);
            }
        }
    }

    return result;
}

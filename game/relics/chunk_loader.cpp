
#include "stdafx.h"
#include "chunk_loader.h"
#include "common_util.h"

#include "block.h"
#include "chunk.h"
#include "game_world.h"
#include "utils.h"
#include "sqlite3.h"


static const std::string DIRT_TOP("dirt_top");
static const std::string STONE_TOP("stone_top");


// Load a chunk from our SQLite file.
// This just deals with the block data. The landscape is are dealt with later.
std::unique_ptr<Chunk> LoadChunk(GameWorld &world, const ChunkOrigin &origin)
{
    PrintDebug("Loading chunk [%d, %d]\n", origin.debugX(), origin.debugZ());

    sqlite3 *db = world.getDatabase();

    std::unique_ptr<char[]> buffer(new char[1024]);

    sprintf(buffer.get(),
        "SELECT x, y, z, block_type FROM blocks "
        "WHERE x >= %d AND x < %d "
        "AND   z >= %d AND z < %d "
        "ORDER BY x, z, y",
        origin.x(), origin.x() + CHUNK_WIDTH,
        origin.z(), origin.z() + CHUNK_WIDTH);
    sqlite3_stmt *stmt = SQL_prepare(db, buffer.get());
    if (stmt == nullptr) {
        assert(false);
        return nullptr;
    }

    std::unique_ptr<Chunk> result = std::make_unique<Chunk>(world, origin);

    // TODO: CONTINUE HERE.
    // Brute force this for now.
    // FUCK: WE NEED TO RESPECT WRITE ORDER.

    int count = 0;
    int ret_code = sqlite3_step(stmt);
    while (ret_code == SQLITE_ROW) {
        int block_x = sqlite3_column_int(stmt, 0);
        int block_y = sqlite3_column_int(stmt, 1);
        int block_z = sqlite3_column_int(stmt, 2);

        const unsigned char *raw_text = sqlite3_column_text(stmt, 3);
        const char *clean = reinterpret_cast<const char*>(raw_text);
        std::string text(clean);

        if (text == DIRT_TOP) {
            GlobalGrid global_coord(block_x, block_y, block_z);
            LocalGrid  local_coord = GlobalGridToLocal(global_coord, origin);

            int local_x = local_coord.x();
            int local_z = local_coord.z();
            for (int y = 0; y < block_y; y++) {
                LocalGrid lookup(local_x, y, local_z);

                // HACK. This will be slow.
                if (result->getBlockType(lookup) == BT_AIR) {
                    result->setBlockType(lookup, BT_DIRT);
                }
                count++;
            }
        }

        else if (text == STONE_TOP) {
            GlobalGrid global_coord(block_x, block_y, block_z);
            LocalGrid  local_coord = GlobalGridToLocal(global_coord, origin);

            int local_x = local_coord.x();
            int local_z = local_coord.z();
            for (int y = 0; y < block_y; y++) {
                LocalGrid lookup(local_x, y, local_z);
                result->setBlockType(lookup, BT_STONE);
                count++;
            }
        }

        ret_code = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);

    return std::move(result);
}


// TODO: Figure this out later.
void SaveChunk(std::unique_ptr<Chunk> chunk)
{
    // All done. Bye bye.
    chunk = nullptr;
}

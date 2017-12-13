
#include "stdafx.h"
#include "chunk_loader.h"
#include "common_util.h"

#include "block.h"
#include "chunk.h"
#include "game_world.h"
#include "utils.h"
#include "sqlite3.h"


// Load a chunk from our SQLite file.
// This just deals with the block data. The landscape is are dealt with later.
Chunk *LoadChunk(GameWorld *pWorld, const ChunkOrigin &origin)
{
    sqlite3 *db = pWorld->getDatabase();

    std::unique_ptr<char[]> buffer(new char[512]);

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

    Chunk *result = new Chunk(pWorld, origin);

    int ret_code = sqlite3_step(stmt);
    while (ret_code == SQLITE_ROW) {
        int block_x = sqlite3_column_int(stmt, 0);
        int block_y = sqlite3_column_int(stmt, 1);
        int block_z = sqlite3_column_int(stmt, 2);

        const unsigned char *raw_text = sqlite3_column_text(stmt, 3);
        std::string text(reinterpret_cast<const char*>(raw_text));

        if (text == "dirt_top") {
            GlobalGrid global_coord(block_x, block_y, block_z);
            LocalGrid  local_coord = GlobalGridToLocal(global_coord, origin);

            int local_x = local_coord.x();
            int local_z = local_coord.z();
            assert((local_x >= 0) && (local_x < CHUNK_WIDTH));
            assert((local_z >= 0) && (local_z < CHUNK_WIDTH));

            for (int y = 0; y < block_y; y++) {
                LocalGrid lookup(local_x, y, local_z);
                result->getBlock(lookup)->setContent(CONTENT_DIRT);
            }
        }

        ret_code = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);

    return result;
}

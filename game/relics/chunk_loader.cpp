
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

    // For all the data in this chunk, use maps to figure out where things start.
    std::map<GlobalPillar, int> dirt_tops;
    std::map<GlobalPillar, int> stone_tops;
    std::map<GlobalPillar, std::vector<int>> coal_spots;

    int count = 0;
    int ret_code = sqlite3_step(stmt);
    while (ret_code == SQLITE_ROW) {
        int x = sqlite3_column_int(stmt, 0);
        int y = sqlite3_column_int(stmt, 1);
        int z = sqlite3_column_int(stmt, 2);

        GlobalPillar pillar(x, z);

        const unsigned char *raw_text = sqlite3_column_text(stmt, 3);
        const char *clean = reinterpret_cast<const char*>(raw_text);
        std::string text(clean);

        if (text == "dirt_top") {
            dirt_tops[pillar] = y;
        }
        else if (text == "stone_top") {
            stone_tops[pillar] = y;
        }
        else if (text == "coal") {
            coal_spots[pillar].emplace_back(y);
        }
        else {
            PrintDebug("Impossible value for block: %s", text);
            assert(false);
        }

        ret_code = sqlite3_step(stmt);
    }

    // Now, build the chunk.
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(world, origin);

    for (const auto &iter : dirt_tops) {
        const GlobalPillar &global_pillar = iter.first;
        LocalPillar &pillar = GlobalPillarToLocal(global_pillar, origin);
        int dirt_top = iter.second;
        for (int y = 0; y <= dirt_top; y++) {
            LocalGrid coord(pillar.x(), y, pillar.z());
            chunk->setBlockType(coord, BT_DIRT);
        }
    }

    for (const auto &iter : stone_tops) {
        const GlobalPillar &global_pillar = iter.first;
        LocalPillar &pillar = GlobalPillarToLocal(global_pillar, origin);
        int stone_top = iter.second;
        for (int y = 0; y <= stone_top; y++) {
            LocalGrid coord(pillar.x(), y, pillar.z());
            chunk->setBlockType(coord, BT_STONE);
        }
    }

    for (const auto &iter : coal_spots) {
        const GlobalPillar &global_pillar = iter.first;
        const std::vector<int> spots_vec  = iter.second;
        LocalPillar &pillar = GlobalPillarToLocal(global_pillar, origin);
        for (int y : spots_vec) {
            LocalGrid coord(pillar.x(), y, pillar.z());
            chunk->setBlockType(coord, BT_COAL);
        }
    }

    // All done.
    return chunk;
}


// TODO: Figure this out later.
void SaveChunk(std::unique_ptr<Chunk> chunk)
{
    // All done. Bye bye.
    chunk = nullptr;
}

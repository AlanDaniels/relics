
#include "stdafx.h"
#include "chunk_loader.h"
#include "common_util.h"

#include "block.h"
#include "chunk.h"
#include "game_world.h"
#include "utils.h"
#include "wavefront_object.h"
#include "sqlite3.h"


static const std::string DIRT_TOP("dirt_top");
static const std::string STONE_TOP("stone_top");


// Load a chunk from our SQLite file.
// This just deals with the block data. The landscape is are dealt with later.
std::unique_ptr<Chunk> LoadChunk(GameWorld &world, const ChunkOrigin &origin)
{
    // Our result.
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(world, origin);

    // A simple test.
    if (origin == ChunkOrigin(0, 0)) {
        MyVec4 move(0, 0, 0);

        std::unique_ptr<WavefrontObject> capsule =
            world.cloneWavefrontObject("capsule", move);

        chunk->addWavefrontObject(std::move(capsule));
    }

    sqlite3 *db = world.getDatabase();

    std::string buffer = fmt::format(
        "SELECT x, y, z, block_type FROM blocks "
        "WHERE x >= {0} AND x < {1} "
        "AND   z >= {2} AND z < {3} "
        "ORDER BY x, z, y",
        origin.x(), origin.x() + CHUNK_WIDTH,
        origin.z(), origin.z() + CHUNK_WIDTH);
    sqlite3_stmt *stmt = SQL_prepare(db, buffer.c_str());
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
            PrintDebug(fmt::format("Impossible value for block: {}", text));
            assert(false);
        }

        ret_code = sqlite3_step(stmt);
    }

    // Now, build the chunk.
    int dirt_top_count = 0;

    for (const auto &iter : dirt_tops) {
        const GlobalPillar &global_pillar = iter.first;
        int dirt_top = iter.second;

        LocalPillar &pillar = GlobalPillarToLocal(global_pillar, origin);
        for (int y = 0; y <= dirt_top; y++) {
            LocalGrid coord(pillar.x(), y, pillar.z());
            chunk->setBlockType(coord, BlockType::DIRT);
        }

        dirt_top_count++;
    }

    for (const auto &iter : stone_tops) {
        const GlobalPillar &global_pillar = iter.first;
        int stone_top = iter.second;

        LocalPillar &pillar = GlobalPillarToLocal(global_pillar, origin);
        for (int y = 0; y <= stone_top; y++) {
            LocalGrid coord(pillar.x(), y, pillar.z());
            chunk->setBlockType(coord, BlockType::STONE);
        }
    }

    for (const auto &iter : coal_spots) {
        const GlobalPillar &global_pillar = iter.first;
        const std::vector<int> spots_vec  = iter.second;

        LocalPillar &pillar = GlobalPillarToLocal(global_pillar, origin);
        for (int y : spots_vec) {
            LocalGrid coord(pillar.x(), y, pillar.z());
            chunk->setBlockType(coord, BlockType::COAL);
        }
    }

    // All done.
    PrintDebug(fmt::format(
        "Loaded chunk [{0}, {1}] with {2} dirt tops.\n", 
        origin.debugX(), origin.debugZ(), dirt_top_count));

    return chunk;
}


// TODO: Figure this out later.
void SaveChunk(std::unique_ptr<Chunk> chunk)
{
    // All done. Bye bye.
    chunk = nullptr;
}

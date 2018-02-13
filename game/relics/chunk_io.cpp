
#include "stdafx.h"
#include "chunk_io.h"
#include "common_util.h"

#include "block.h"
#include "chunk.h"
#include "game_world.h"
#include "resource_pool.h"
#include "wavefront_object.h"
#include "utils.h"

#include "sqlite3.h"


static const std::string DIRT_TOP("dirt_top");
static const std::string STONE_TOP("stone_top");


// TEMP: C++ is fucking impossible at times.
std::unique_ptr<Chunk> FuckYou(const std::string &blah, GameWorld *world) {
    return nullptr;
}


// Get the player's start position.
// TODO: For now, just place them at the dirt top of the block at X=0, Z=0.
MyVec4 GetPlayerStartPos(const std::string &db_fname)
{
    MyVec4 never_mind(0, 0, 0);

    sqlite3 *db = SQL_open(db_fname);
    if (db == nullptr) {
        PrintDebug(fmt::format("Could not open DB '{}'", db_fname));
        return never_mind;
    }

    std::string buffer =
        "SELECT y FROM blocks "
        "WHERE x == 0 AND z == 0 AND block_type = 'dirt_top'";
    sqlite3_stmt *stmt = SQL_prepare(db, buffer.c_str());
    if (stmt == nullptr) {
        assert(false);
        return never_mind;
    }

    int y;
    int ret_code = sqlite3_step(stmt);
    if (ret_code == SQLITE_ROW) {
        y = sqlite3_column_int(stmt, 0);
    }
    else {
        y = 0;
    }

    // Add one, since we're on top of the block.
    y++;

    // All done.
    SQL_finalize(db, stmt);
    SQL_close(db);

    GLfloat half_x   = BLOCK_SCALE / 2;
    GLfloat scaled_y = y * BLOCK_SCALE;
    GLfloat half_z   = BLOCK_SCALE / 2;
    return MyVec4(half_x, scaled_y, half_z);
}


// Load a chunk from our SQLite file.
// This just deals with the block data. The landscape is are dealt with later.
// For the world, don't touch the reference, just save it.
std::unique_ptr<Chunk> LoadChunk(const std::string &db_fname, GameWorld *world, const ChunkOrigin &origin)
{
    // Our result.
    std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>(*world, origin);

    // TODO: A simple test of a Wavefront Object.
    if (origin == ChunkOrigin(0, 0)) {
        MyVec4 move(0, 0, 0);

        const auto &pool = GetResourcePool();
        std::unique_ptr<WFInstance> capsule = pool.cloneWFObject("capsule", move);
        chunk->addWFInstance(std::move(capsule));
    }

    sqlite3 *db = SQL_open(db_fname);
    if (db == nullptr) {
        PrintDebug(fmt::format("Could not open DB '{}'", db_fname));
        return nullptr;
    }

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

    SQL_finalize(db, stmt);

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

    SQL_close(db);

    return chunk;
}


// TODO: Figure this out later.
void SaveChunk(GameWorld &world, std::unique_ptr<Chunk> chunk)
{
    // All done. Bye bye.
    chunk = nullptr;
}

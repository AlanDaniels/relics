
#include "stdafx.h"
#include "game_world.h"

#include "chunk.h"
#include "chunk_io.h"
#include "common_util.h"
#include "config.h"
#include "event_handler.h"
#include "draw_state_pct.h"
#include "draw_state_pt.h"
#include "hit_test_result.h"
#include "physics.h"
#include "player.h"
#include "my_math.h"
#include "utils.h"

#include "sqlite3.h"


// Our game world.
GameWorld::GameWorld(const std::string &db_fname) :
    m_db_fname(db_fname),
    m_paused(false),
    m_game_time_msecs(0),
    m_time_since_worker_msecs(0),
    m_player(std::make_unique<Player>(*this))
{
    setPlayerAtStart();

    // Figure out the size of our drawing region.
    // We load one border larger than what we will actually draw.
    int eval_block_count = GetConfig().logic.eval_block_count;

    auto player_pos    = m_player->getPlayerPos();
    auto draw_region   = WorldPosToEvalRegion(player_pos, eval_block_count);
    auto bigger_region = draw_region.expand();

    // Fire off a thread for each chunk we want to load. 
    std::vector<ChunkFuture> future_vec;
    int count = 0;

    for (const auto &origin : bigger_region.getEntirety()) {
        auto loader_future = std::async(std::launch::async, LoadChunk, m_db_fname, this, origin);
        future_vec.push_back(std::move(loader_future));
        count++;
    }

    PrintDebug(fmt::format("Spawned {} threads to load chunks. Waiting for them to complete...\n", count));

    // Wait for each thread to complete.
    for (auto &future : future_vec) {
        std::unique_ptr<Chunk> chunk = future.get();
        chunk->touch(m_game_time_msecs);
        const ChunkOrigin &origin = chunk->getOrigin();
        m_chunk_map[origin] = std::move(chunk);
    }

    PrintDebug("Threads are completed.\n");

    // Now that all the chunks are loaded, finish up any last calculations.
    for (auto &iter : m_chunk_map) {
        Chunk *chunk = iter.second.get();

        SurfaceTotals totals;
        chunk->rebuildInnerExposedBlockSet(&totals);
        chunk->rebuildEdgeExposedBlockSet(&totals);
        chunk->rebuildLandscape();
    }
}


// Game world destructor.
GameWorld::~GameWorld()
{
    for (auto &iter : m_chunk_map) {
        ChunkOrigin origin = iter.first;
        m_chunk_map[origin] = nullptr;
    }
}


// Figure out the starting position of the player.
// For now, smack in the center of the starting chunk,
// and one quarter of the way up.
void GameWorld::setPlayerAtStart()
{
    MyVec4 start = GetPlayerStartPos(m_db_fname);
    m_player->setPlayerPos(start);
    m_player->setCameraPitch(0.0f);
    m_player->setCameraYaw(0.0f);
    m_current_grid_coord   = WorldPosToGlobalGrid(start, NudgeType::NONE);
    m_current_chunk_origin = WorldToChunkOrigin(start);
}


// Look up a chunk in our world.
// It's possible that this hasn't been loaded yet.
const Chunk *GameWorld::getChunk(const ChunkOrigin &origin) const
{
    if (!IS_KEY_IN_MAP(m_chunk_map, origin)) {
        return nullptr;
    }

    auto iter = m_chunk_map.find(origin);
    const Chunk *pResult = iter->second.get();
    assert(pResult != nullptr);
    return pResult;
}



// Handle a game tick. This should always be called through the 
// "GameEventHandler" object, which will deal with keyboard events.
void GameWorld::onGameTick(int elapsed_msec, const EventStateMsg &msg)
{
    // If we're paused, nothing to do here.
    if (m_paused) {
        return;
    }

    // Update everything in the world.
    m_player->onGameTick(elapsed_msec, msg);
    PlayerCollisionTest(*m_player, elapsed_msec);

    // Since the player moved, see if we need to recalc any of the world.
    MyVec4      camera_pos   = m_player->getCameraPos();
    GlobalGrid  new_location = WorldPosToGlobalGrid(camera_pos, NudgeType::NONE);

    int eval_block_count = GetConfig().logic.eval_block_count;
    ChunkOrigin new_chunk_origin = WorldToChunkOrigin(camera_pos);
    if (new_chunk_origin != m_current_chunk_origin) {
        loadWorldAsNeeded();
    }

    m_current_grid_coord   = new_location;
    m_current_chunk_origin = new_chunk_origin;

    // If it's been long enough, cash in a worker thread.
    if (m_time_since_worker_msecs > WORKER_PACE_MSECS) {
        cashInWorkerThread();
        m_time_since_worker_msecs = 0;
    }

    // Recalc our hit test, and we're done.
    calcHitTest();
    m_game_time_msecs += elapsed_msec;
    m_time_since_worker_msecs += elapsed_msec;
}


// Load any new game chunks that we need. The logic here is tricky, and very thread heavy!
void GameWorld::loadWorldAsNeeded()
{
    const MyVec4 &camera_pos = m_player->getCameraPos();

    int eval_block_count = GetConfig().logic.eval_block_count;
    EvalRegion draw_region = WorldPosToEvalRegion(camera_pos, eval_block_count);

    // For anything inside the draw region, if we don't have it loaded already,
    // we damn well better have a future waiting for us, and we'll check that in.
    std::vector<ChunkOrigin> origins = draw_region.getEntirety();
    for (const auto &origin : origins) {
        bool already_loaded = IS_KEY_IN_MAP(m_chunk_map, origin);
        if (!already_loaded) {
            bool now_arriving = IS_KEY_IN_MAP(m_chunk_loader_map, origin);
            assert(now_arriving);

            auto arrival = m_chunk_loader_map.find(origin);
            std::unique_ptr<Chunk> chunk = arrival->second.get();
            m_chunk_map[origin] = std::move(chunk);

            m_chunk_loader_map.erase(arrival);
        }
    }

    // Next, recalc the chunks within the draw region.
    // The block of code above should have everything loaded already.
    SurfaceTotals ignored;

    for (const auto &origin : draw_region.getEntirety()) {
        assert(IS_KEY_IN_MAP(m_chunk_map, origin));

        auto &chunk = m_chunk_map.at(origin);
        switch (chunk->getStatus()) {
        case ChunkStatus::INNER:
            chunk->rebuildEdgeExposedBlockSet(&ignored);
            chunk->rebuildLandscape();
            break;

        case ChunkStatus::UP_TO_DATE:
            break;

        default:
            PrintTheImpossible(__FILE__, __LINE__, static_cast<int>(chunk->getStatus()));
            break;
        }
    }

    // For any chunk just beyond our view, if it's not loaded already, fire off
    // a thread to load it. If there aleady *is* such a thread, then never mind.
    EvalRegion edge_region = draw_region.expand();

    for (const auto &origin : edge_region.getOutline()) {
        bool already_loaded = IS_KEY_IN_MAP(m_chunk_map, origin);
        if (!already_loaded) {
            bool already_queued = IS_KEY_IN_MAP(m_chunk_loader_map, origin);
            if (!already_queued) {
                auto &loader_future = std::async(std::launch::async, LoadChunk, m_db_fname, this, origin);
                m_chunk_loader_map[origin] = std::move(loader_future);
            }
        }
    }

    // Okay! With that, time to do some housecleaning. If a chunk has expired,
    // unload it, and pass it off to a thread to save its contents.
    // BIG TODO: Don't hard-code the expiration time.
    const int EXPIRATION_TIME_MSECS = 5000;

    for (auto &iter : m_chunk_map) {
        ChunkOrigin origin = iter.first;

        if (!draw_region.contains(origin)) {
            Chunk &chunk = *iter.second;
            int last_touched = m_game_time_msecs - chunk.getLastTouchedMsecs();
            if (last_touched > EXPIRATION_TIME_MSECS) {
                chunk.landscape.freeSurfaceLists();

                // BIG TODO: Add logic to save the chunk here.

                // We can't delete keys as we walk the map.
                // Instead, set a null, and wait for the next block.
                m_chunk_map[origin] = nullptr;
            }
        }
    }

    // Now that we've let go of any expired chunks, clear our any nulls
    // out of our chunk map. The "std::map" type has no filtering that doesn't
    // involve horrible template errors, so we have to get a bit clever.
    auto iter = m_chunk_map.begin();
    while (iter != m_chunk_map.end()) {
        if (iter->second == nullptr) {
            iter = m_chunk_map.erase(iter);
        }
        else {
            ++iter;
        }
    }
}


// Calc our hit test, only in the eval region.
void GameWorld::calcHitTest()
{
    bool success = false;

    MyRay  camera_ray = m_player->getCameraRay();
    MyVec4 camera_pos = m_player->getCameraPos();

    GLfloat best_distance = FLT_MAX;
    Chunk *best_chunk = nullptr;
    HitTestResult best_detail;

    GLfloat hit_test_distance = GetConfig().logic.getHitTestDistanceCm();
    int block_count = static_cast<int>((hit_test_distance / 100.0f) / CHUNK_WIDTH);
    EvalRegion region = WorldPosToEvalRegion(camera_pos, block_count);

    for     (int x = region.west();  x <= region.east();  x += CHUNK_WIDTH) {
        for (int z = region.south(); z <= region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            Chunk *chunk = m_chunk_map[origin].get();

            HitTestResult detail;
            bool this_test = DoChunkHitTest(*chunk, camera_ray, &detail);
            if (this_test && (detail.getDist() < best_distance)) {
                success       = true;
                best_distance = detail.getDist();
                best_chunk    = chunk;
                best_detail   = std::move(detail);
            }
        }
    }

    // If we weren't close enough, then never mind.
    if (best_distance > hit_test_distance) {
        success = false;
    }

    // Once we're done, *then* rebuild the quad list.
    m_hit_test_vert_list.reset();
    if (success) {
#if 0
        ChunkHitTestToQuad(*best_chunk, best_detail, &m_hit_test_vert_list);
#endif
    }
    m_hit_test_vert_list.update();

    m_hit_test_success = success;
    if (success) {
        m_hit_test_result = std::move(best_detail);
    }
    else {
        HitTestResult blank;
        m_hit_test_result = std::move(blank);
    }
}


// DEBUG: Obviously we want more graceful logic here!
// Recalculating just 9 stripes vs all of them. But hey, it's a start.
void GameWorld::deleteBlockInFrontOfUs()
{
    if (m_hit_test_success) {
        const auto &origin       = m_hit_test_result.getChunkOrigin();
        const auto &global_coord = m_hit_test_result.getGlobalCoord();
        const auto &local_coord  = GlobalGridToLocal(global_coord, origin);

        const auto &iter = m_chunk_map.find(origin);
        Chunk *chunk = iter->second.get();

        assert(chunk != nullptr);
        assert(chunk->IsGlobalGridWithin(global_coord));

        chunk->setBlockType(local_coord, BlockType::AIR);

        SurfaceTotals totals;
        chunk->rebuildInnerExposedBlockSet(&totals);
        chunk->rebuildEdgeExposedBlockSet(&totals);
        chunk->rebuildLandscape();
    }
}


// Cash in a worker thread.
// Return true if there was anything to do.
bool GameWorld::cashInWorkerThread() {
    // Ignore the surface totals for this part.
    SurfaceTotals totals;

    // Check our arrival threads, and find any completed futures.
    for (auto &iter : m_chunk_loader_map) {
        auto &origin  = iter.first;
        auto &arrival = iter.second;

        if (IsFutureReady(arrival)) {
            // We should *not* have this chunk already.
            assert(!IS_KEY_IN_MAP(m_chunk_map, origin));

            std::unique_ptr<Chunk> chunk = arrival.get();
            chunk->rebuildInnerExposedBlockSet(&totals);
            chunk->rebuildEdgeExposedBlockSet(&totals);
            chunk->rebuildLandscape();
            m_chunk_map[origin] = std::move(chunk);

            // All done.
            auto whatever = m_chunk_loader_map.find(origin);
            m_chunk_loader_map.erase(whatever);
            return true;
        }
    }

    // Nothing to do.
    return false;
}
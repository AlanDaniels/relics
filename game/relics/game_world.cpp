
#include "stdafx.h"
#include "game_world.h"

#include "chunk.h"
#include "chunk_loader.h"
#include "common_util.h"
#include "config.h"
#include "event_handler.h"
#include "draw_state_pct.h"
#include "draw_state_pt.h"
#include "hit_test_result.h"
#include "player.h"
#include "my_math.h"
#include "utils.h"

#include "sqlite3.h"


// Our game world.
GameWorld::GameWorld(const std::string &db_fname) :
    m_db_fname(db_fname),
    m_paused(false),
    m_time_msec(0),
    m_player(std::make_unique<Player>())
{
    setPlayerAtStart();

    // Figure out the size of our drawing region.
    // We load one border larger than what we will actually draw.
    int eval_blocks = GetConfig().logic.eval_blocks;

    MyVec4 camera_pos = m_player->getCameraPos();
    EvalRegion draw_region = WorldPosToEvalRegion(camera_pos, eval_blocks);
    EvalRegion load_region = draw_region.expand();

    // TODO: Forget threads for now. 
    int count = 0;
    for     (int x = load_region.west();  x <= load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z <= load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            m_chunk_map[origin] = LoadChunk(*this, origin);
            count++;
        }
    }

    PrintDebug(fmt::format("Created {} chunks.\n", count));

    // Recalc and realize every chunk that we've loaded.
    // This leaves a ring of unrealized chunks around us, and that's okay.
    for     (int x = load_region.west();  x <= load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z <= load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            m_chunk_map[origin].get()->recalcAllExposures();
        }
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
    m_player->setPos(start);
    m_current_grid_coord   = WorldPosToGlobalGrid(start, NudgeType::NONE);
    m_current_chunk_origin = WorldToChunkOrigin(start);
}


// Look up a chunk that absolutely must have been loaded already,
// such as the player position, or the current drawing area.
const Chunk *GameWorld::getRequiredChunk(const ChunkOrigin &origin) const
{
    auto iter = m_chunk_map.find(origin);
    assert(iter != m_chunk_map.end());

    const Chunk *result = iter->second.get();
    assert(result != nullptr);
    return result;
}


// Look up a chunk that may not necessarily be loaded yet.
const Chunk *GameWorld::getOptionalChunk(const ChunkOrigin &origin) const
{
    auto iter = m_chunk_map.find(origin);
    if (iter == m_chunk_map.end()) {
        return nullptr;
    }

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

    // Since the player moved, see if we need to recalc any of the world.
    MyVec4 camera_pos = m_player->getCameraPos();
    GlobalGrid  new_location = WorldPosToGlobalGrid(camera_pos, NudgeType::NONE);

    int eval_blocks = GetConfig().logic.eval_blocks;
    ChunkOrigin new_chunk_origin = WorldToChunkOrigin(camera_pos);
    if (new_chunk_origin != m_current_chunk_origin) {
        updateWorld();
    }

    m_current_grid_coord   = new_location;
    m_current_chunk_origin = new_chunk_origin;

    // Recalc our hit test, and we're done.
    calcHitTest();
    m_time_msec += elapsed_msec;
}


// Update the world. The logic here is tricky, and very thread heavy!
void GameWorld::updateWorld()
{
    const MyVec4 &camera_pos = m_player->getCameraPos();

    int eval_blocks = GetConfig().logic.eval_blocks;
    EvalRegion draw_region = WorldPosToEvalRegion(camera_pos, eval_blocks);
    EvalRegion load_region = draw_region.expand();

    // For any chunk at the "edge" of the load region, load it if we don't have it already.
    for     (int x = load_region.west();  x <= load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z <= load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);

            if (!draw_region.contains(origin)) {
                if (m_chunk_map.find(origin) == m_chunk_map.end()) {
                    m_chunk_map[origin] = LoadChunk(*this, origin);
                    m_chunk_map[origin]->recalcAllExposures();
                }
            }
        }
    }

    // Then, recalc the chunks within the draw region.
    for     (int x = draw_region.west();  x <= draw_region.east();  x += CHUNK_WIDTH) {
        for (int z = draw_region.south(); z <= draw_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            auto &iter = m_chunk_map.find(origin);

            // The "edge" case above should have loaded this chunk already.
            assert(iter != m_chunk_map.end());
            Chunk &chunk = *iter->second;

            if (!chunk.isUpToDate()) {
                chunk.recalcAllExposures();
            }
        }
    }

    // Once that's done, do some housecleaning.
    for (auto &it : m_chunk_map) {
        ChunkOrigin origin = it.first;

#if 0
        // If a chunk is outside our drawing region,
        // unrealize it to take it easier on the video card.
        if (!draw_region.contains(origin)) {
            Chunk &chunk = *it.second;
            chunk.unrealizeSurfaceLists();
        }
#endif

        // If a chunk is outside or loading region,
        // save its contents and delete it.
        if (!load_region.contains(origin)) {
            SaveChunk(*this, std::move(it.second));
        }
    }

    // Clear our any nulls. The "std::map" type has no filtering that
    // doesn't involve horrible template errors, so we have to get clever.
    auto it = m_chunk_map.begin();
    while (it != m_chunk_map.end()) {
        if (it->second == nullptr) {
            it = m_chunk_map.erase(it);
        }
        else {
            ++it;
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
        chunk->recalcAllExposures();
    }
}

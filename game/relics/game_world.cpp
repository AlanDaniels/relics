
#include "stdafx.h"
#include "game_world.h"

#include "chunk.h"
#include "chunk_hit_test.h"
#include "chunk_loader.h"
#include "config.h"
#include "event_handler.h"
#include "draw_state_pct.h"
#include "draw_state_pt.h"
#include "my_math.h"
#include "utils.h"


// TODO: Add the final part: Background threads as we move around the map.
// Touch each chunk when it's drawn. After we haven't seen a chunk in say, one minute, we can unload it. Simpler than an LRU list.
// Also, stop using references to ownership. Use pointers instead. More sane that way.


// Our game world.
GameWorld::GameWorld() :
    m_paused(false),
    m_time_msec(0),
    m_msecs_since_worker(0),
    m_camera_pos(getCameraStartPos()),
    m_camera_pitch(0.0f),
    m_camera_yaw(0.0f)
{
    // Figure out the size of our drawing region.
    // We load one border larger than what we will actually draw.
    int        eval_blocks = GetConfig().logic.eval_blocks;
    EvalRegion draw_region = WorldToEvalRegion(m_camera_pos, eval_blocks);
    EvalRegion load_region = draw_region.expand();

    // Fire off a thread to load each chunk we need. 
    std::vector<t_chunk_future> future_list;

    for     (int x = load_region.west();  x < load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z < load_region.north(); z += CHUNK_WIDTH) {
            t_chunk_future fut = std::async(std::launch::async, LoadChunkAsync, this, ChunkOrigin(x, z));
            future_list.emplace_back(std::move(fut));
        }
    }

    // Wait for each thread to complete.
    int count = 0;
    for (auto iter = future_list.begin(); iter != future_list.end(); iter++) {
        Chunk *pChunk = iter->get();
        ChunkOrigin origin = pChunk->getOrigin();
        m_chunk_map[origin] = pChunk;
        count++;
    }

    PrintDebug("Created %d chunks.\n", count);

    // Recalc and realize every chunk that we've loaded.
    // This leaves a ring of unrealized chunks around us, and that's okay.
    for     (int x = load_region.west();  x < load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z < load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            Chunk *pChunk = m_chunk_map[origin];
            pChunk->recalcLandscape();
            pChunk->realizeLandscape();
        }
    }
}


// Game world destructor.
GameWorld::~GameWorld()
{
    // TODO: What to do here? Finish out threads I guess.
}


// Figure out where the camera will start.
// For now, smack in the center of the starting chunk,
// and one quarter of the way up.
MyVec4 GameWorld::getCameraStartPos() const
{
    GLfloat x = BLOCK_SCALE * CHUNK_WIDTH  * 0.5f;
    GLfloat y = BLOCK_SCALE * CHUNK_HEIGHT * 0.25f;
    GLfloat z = BLOCK_SCALE * CHUNK_WIDTH  * 0.5f;
    return MyVec4(x, y, z);
}


// Reset the camera.
void GameWorld::resetCamera()
{
    m_camera_pitch = 0.0f;
    m_camera_yaw   = 0.0f;
    m_camera_pos   = getCameraStartPos();
    m_current_grid_coord   = WorldToGridCoord(m_camera_pos, NUDGE_NONE);
    m_current_chunk_origin = WorldToChunkOrigin(m_camera_pos);
}


// Get the chunk the player is standing in.
// This should never be null. Otherwise, how would we be here?
const Chunk *GameWorld::getPlayersChunk() const
{
    auto iter = m_chunk_map.find(m_current_chunk_origin);
    assert(iter != m_chunk_map.end());

    const Chunk *pResult = iter->second;
    assert(pResult != nullptr);
    return pResult;
}


// Look up a chunk that absolutely must have been loaded already,
// such as the player position, or the current drawing area.
const Chunk *GameWorld::getRequiredChunk(const ChunkOrigin &origin) const
{
    auto iter = m_chunk_map.find(origin);
    assert(iter != m_chunk_map.end());

    const Chunk *result = iter->second;
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

    const Chunk *pResult = iter->second;
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

    // Every few seconds, cash in a worker thread.
    m_msecs_since_worker += elapsed_msec;
    if (m_msecs_since_worker >= WORKER_PACE_MSECS) {
        cashInWorkerThread();
        m_msecs_since_worker = 0;
    }

    GLfloat degrees_per_pixel = GetConfig().window.mouse_degrees_per_pixel;

    // Handle the mouse movement.
    int mouse_diff_x = msg.getMouseDiffX();
    int mouse_diff_y = msg.getMouseDiffY();
    m_camera_yaw   += static_cast<GLfloat>(mouse_diff_x) * degrees_per_pixel;
    m_camera_pitch -= static_cast<GLfloat>(mouse_diff_y) * degrees_per_pixel;
    clampRotations();

    // Then, handle the camera movement.
    // TODO: The math here is tricky, and should we tie it to the elapsed time?
    GLfloat meters_per_sec = GetConfig().debug.noclip_flight_speed;
    GLfloat centimeters    = (meters_per_sec / 10.0f) * elapsed_msec;

    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);
    MyMatrix4by4 rotator = roty.times(rotx);

    GLfloat boost = msg.getSpeedBoost() ? 10.0f : 1.0f;

    if (msg.getMoveFwd()) {
        MyVec4 rotated = rotator.times(VEC4_NORTHWARD);
        MyVec4 move    = rotated.times(centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move.x(), move.y(), move.z());
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveBkwd()) {
        MyVec4 rotated = rotator.times(VEC4_NORTHWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move.x(), move.y(), move.z());
        m_camera_pos = tr.times(m_camera_pos);
    }

    if (msg.getMoveLeft()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move.x(), move.y(), move.z());
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveRight()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move.x(), move.y(), move.z());
        m_camera_pos = tr.times(m_camera_pos);
    }

    if (msg.getMoveUp()) {
        MyVec4 move = VEC4_UPWARD.times(centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move.x(), move.y(), move.z());
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveDown()) {
        MyVec4 move = VEC4_UPWARD.times(-centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move.x(), move.y(), move.z());
        m_camera_pos = tr.times(m_camera_pos);
    }

    // Since our camera moved, see if we need to recalc any of the world.
    MyGridCoord  new_location = WorldToGridCoord(m_camera_pos, NUDGE_NONE);

    int eval_blocks = GetConfig().logic.eval_blocks;
    ChunkOrigin new_chunk_origin = WorldToChunkOrigin(m_camera_pos);
    if (new_chunk_origin != m_current_chunk_origin) {
        updateWorld();
    }

    m_current_grid_coord   = new_location;
    m_current_chunk_origin = new_chunk_origin;

    // Recalc our hit test, and we're done.
    calcHitTest();
    m_time_msec += elapsed_msec;
}


// Get the camera's eye-ray. Eh, maybe we could cache this.
MyRay GameWorld::getCameraEyeRay() const
{
    MyMatrix4by4 roty = MyMatrix4by4::RotateY( m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);
    MyMatrix4by4 rotator = roty.times(rotx);

    MyVec4 eye_normal = rotator.times(VEC4_NORTHWARD);
    return MyRay(m_camera_pos, eye_normal);
}


// Update the world. The logic here is tricky, and very thread heavy!
void GameWorld::updateWorld()
{
    int eval_blocks = GetConfig().logic.eval_blocks;
    EvalRegion draw_region = WorldToEvalRegion(m_camera_pos, eval_blocks);
    EvalRegion load_region = draw_region.expand();

    // For any chunk at the "edge" of the load region, spawn a worker threads to get it loading.
    for     (int x = load_region.west();  x < load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z < load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);

            if (NOT(draw_region.containsOrigin(origin))) {
                // See if we have the chunk loaded yet.
                // No? Then see if we have a thread going for it.
                // Still no? Then fire off a thread to load it.
                if (m_chunk_map.find(origin) == m_chunk_map.end()) {
                    if (m_load_future_map.find(origin) == m_load_future_map.end()) {
                        m_load_future_map[origin] =
                            std::async(std::launch::async, LoadChunkAsync, this, origin);
                    }
                }
            }
        }
    }

    // Then, only recalc the chunks within the draw region.
    // If the chunk hasn't loaded yet, then alas, wait for it's loaded to complete.
    // Needless to say, this isn't ideal (stutter!), so avoid this as much as possible.
    // Also, we damn well better have a thread going for this already.
    for     (int x = draw_region.west();  x < draw_region.east();  x += CHUNK_WIDTH) {
        for (int z = draw_region.south(); z < draw_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);

            if (m_chunk_map.find(origin) == m_chunk_map.end()) {
                auto future_iter = m_load_future_map.find(origin);
                assert(future_iter != m_load_future_map.end());

                Chunk *pFreshChunk = future_iter->second.get();
                m_load_future_map.erase(origin);
                m_chunk_map[origin] = pFreshChunk;
            }

            // Finally, rebuild the landscape as needed.
            Chunk *pChunk = m_chunk_map[origin];
            if (NOT(pChunk->isLandscapeCurrent())) {
                pChunk->recalcLandscape();
            }
            if (NOT(pChunk->isLandcsapeRealized())) {
                pChunk->realizeLandscape();
            }
        }
    }
}


// Clamp the camera rotations.
void GameWorld::clampRotations()
{
    while (m_camera_yaw > 180.0) {
        m_camera_yaw -= 360.0;
    }
    while (m_camera_yaw <= -180.0) {
        m_camera_yaw += 360.0;
    }

    if (m_camera_pitch > 90.0) {
        m_camera_pitch = 90.0;
    }
    else if (m_camera_pitch < -90.0) {
        m_camera_pitch = -90.0;
    }
}


// Calc our hit test, only in the eval region.
void GameWorld::calcHitTest()
{
    bool success = false;
    MyRay eye_ray = getCameraEyeRay();

    GLfloat best_distance = FLT_MAX;
    Chunk *best_chunk = nullptr;
    ChunkHitTestDetail best_detail;

    GLfloat hit_test_distance = GetConfig().logic.getHitTestDistanceCm();
    int block_count = static_cast<int>((hit_test_distance / 100.0f) / CHUNK_WIDTH);
    EvalRegion region = WorldToEvalRegion(m_camera_pos, block_count);

    for     (int x = region.west();  x < region.east();  x += CHUNK_WIDTH) {
        for (int z = region.south(); z < region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            Chunk *pChunk = m_chunk_map[origin];

            ChunkHitTestDetail detail;
            bool this_test = DoChunkHitTest(*pChunk, eye_ray, &detail);
            if (this_test && (detail.getDist() < best_distance)) {
                success       = true;
                best_distance = detail.getDist();
                best_chunk    = pChunk;
                best_detail   = detail;
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
        ChunkHitTestToQuad(*best_chunk, best_detail, &m_hit_test_vert_list);
    }
    m_hit_test_vert_list.realize();

    m_hit_test_success = success;
    if (success) {
        m_hit_test_detail = best_detail;
    }
    else {
        ChunkHitTestDetail blank;
        m_hit_test_detail = blank;
    }
}


// DEBUG: Obviously we want more graceful logic here!
// Recalculating just 9 stripes vs all of them. But hey, it's a start.
void GameWorld::deleteBlockInFrontOfUs()
{
    if (m_hit_test_success) {
        ChunkOrigin origin = m_hit_test_detail.getChunkOrigin();
        MyGridCoord coord  = m_hit_test_detail.getGlobalCoord();

        const auto &iter = m_chunk_map.find(origin);
        Chunk *pChunk = iter->second;

        assert(pChunk != nullptr);
        assert(pChunk->isCoordWithin(coord));

        pChunk->getBlockGlobal_RW(coord)->setContent(CONTENT_AIR);
        pChunk->recalcLandscape();
        pChunk->realizeLandscape();
    }
}


// Every couple of seconds, cash in a worker thread.
void GameWorld::cashInWorkerThread()
{
    // Nothing to do? Bail out, so that I can set breakpoints easier.
    if (m_load_future_map.size() == 0) {
        return;
    }

    // Only look for one thread to finish, in no particular order.
    for (auto iter = m_load_future_map.begin(); iter != m_load_future_map.end(); iter++) {
        if (IsFutureReady<Chunk *>(iter->second)) {
            Chunk *fresh_chunk  = iter->second.get();
            ChunkOrigin origin  = fresh_chunk->getOrigin();
            m_chunk_map[origin] = fresh_chunk;
            m_load_future_map.erase(origin);
            PrintDebug("Thread to load chunk [%d, %d] is complete.\n", origin.x(), origin.z());
            return;
        }
    }
}

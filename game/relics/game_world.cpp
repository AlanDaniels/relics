
#include "stdafx.h"
#include "game_world.h"
#include "common_util.h"

#include "chunk.h"
#include "chunk_hit_test.h"
#include "chunk_loader.h"
#include "config.h"
#include "event_handler.h"
#include "draw_state_pct.h"
#include "draw_state_pt.h"
#include "my_math.h"
#include "utils.h"

#include "sqlite3.h"


// Our game world.
GameWorld::GameWorld(sqlite3 *db) :
    m_database(db),
    m_paused(false),
    m_time_msec(0),
    m_camera_pitch(0.0f),
    m_camera_yaw(0.0f)
{
    // Figure out the size of our drawing region.
    // We load one border larger than what we will actually draw.
    int eval_blocks = GetConfig().logic.eval_blocks;

    EvalRegion draw_region = WorldPosToEvalRegion(m_camera_pos, eval_blocks);
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

    m_camera_pos = getCameraStartPos();
}


// Game world destructor.
GameWorld::~GameWorld()
{
    sqlite3_close(m_database);
    for (auto &iter : m_chunk_map) {
        ChunkOrigin origin = iter.first;
        m_chunk_map[origin] = nullptr;
    }
}


// Figure out where the camera will start.
// For now, smack in the center of the starting chunk,
// and one quarter of the way up.
MyVec4 GameWorld::getCameraStartPos() const
{
    GLfloat x = BLOCK_SCALE * CHUNK_WIDTH  * 0.5f;
    GLfloat y = BLOCK_SCALE * CHUNK_HEIGHT * 0.1f;
    GLfloat z = BLOCK_SCALE * CHUNK_WIDTH  * 0.5f;

#if 0
    // TODO: Find the top level.
    ChunkOrigin origin(0, 0);
    m_chunk_map[origin] @@@@;
#endif
    return MyVec4(x, y, z);
}


// Reset the camera.
void GameWorld::resetCamera()
{
    m_camera_pitch = 0.0f;
    m_camera_yaw   = 0.0f;
    m_camera_pos   = getCameraStartPos();
    m_current_grid_coord   = WorldPosToGlobalGrid(m_camera_pos, NudgeType::NONE);
    m_current_chunk_origin = WorldToChunkOrigin(m_camera_pos);
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

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveBkwd()) {
        MyVec4 rotated = rotator.times(VEC4_NORTHWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    if (msg.getMoveLeft()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveRight()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    if (msg.getMoveUp()) {
        MyVec4 move = VEC4_UPWARD.times(centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveDown()) {
        MyVec4 move = VEC4_UPWARD.times(-centimeters * boost);

        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    // Since our camera moved, see if we need to recalc any of the world.
    GlobalGrid  new_location = WorldPosToGlobalGrid(m_camera_pos, NudgeType::NONE);

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
    EvalRegion draw_region = WorldPosToEvalRegion(m_camera_pos, eval_blocks);
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
            SaveChunk(std::move(it.second));
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
    EvalRegion region = WorldPosToEvalRegion(m_camera_pos, block_count);

    for     (int x = region.west();  x <= region.east();  x += CHUNK_WIDTH) {
        for (int z = region.south(); z <= region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            Chunk *chunk = m_chunk_map[origin].get();

            ChunkHitTestDetail detail;
            bool this_test = DoChunkHitTest(*chunk, eye_ray, &detail);
            if (this_test && (detail.getDist() < best_distance)) {
                success       = true;
                best_distance = detail.getDist();
                best_chunk    = chunk;
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
#if 0
        ChunkHitTestToQuad(*best_chunk, best_detail, &m_hit_test_vert_list);
#endif
    }
    m_hit_test_vert_list.update();

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
        ChunkOrigin origin       = m_hit_test_detail.getChunkOrigin();
        GlobalGrid  global_coord = m_hit_test_detail.getGlobalCoord();
        LocalGrid   local_coord  = GlobalGridToLocal(global_coord, origin);

        const auto &iter = m_chunk_map.find(origin);
        Chunk *chunk = iter->second.get();

        assert(chunk != nullptr);
        assert(chunk->IsGlobalGridWithin(global_coord));

        chunk->setBlockType(local_coord, BlockType::AIR);
        chunk->recalcAllExposures();
    }
}

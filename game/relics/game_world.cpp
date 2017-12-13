
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
#include <boost/filesystem.hpp>


// TODO: Add the final part: Background threads as we move around the map.
// Touch each chunk when it's drawn. After we haven't seen a chunk in say, one minute, we can unload it. Simpler than an LRU list.
// Also, stop using references to ownership. Use pointers instead. More sane that way.


// Initialize the game world.
// There's one per database file, and if we can't open it, there's no world.
GameWorld *GameWorld::Create() 
{
    std::string fname = GetConfig().world.file_name;
    std::string full_name = RESOURCE_PATH;
    full_name.append(fname);

    if (!boost::filesystem::is_regular_file(full_name)) {
        PrintDebug("Database file %s does not exist.", full_name.c_str());
        return nullptr;
    }

    sqlite3 *database = SQL_open(full_name);
    if (database == nullptr) {
        return nullptr;
    }

    return new GameWorld(database);
}


// Our game world.
GameWorld::GameWorld(sqlite3 *database) :
    m_database(database),
    m_paused(false),
    m_time_msec(0),
    m_camera_pitch(0.0f),
    m_camera_yaw(0.0f),
    m_camera_pos(getCameraStartPos())
{
    // Figure out the size of our drawing region.
    // We load one border larger than what we will actually draw.
    int        eval_blocks = GetConfig().logic.eval_blocks;
    EvalRegion draw_region = WorldToEvalRegion(m_camera_pos, eval_blocks);
    EvalRegion load_region = draw_region.expand();

    // TODO: Forget threads for now. 
    int count = 0;
    for     (int x = load_region.west();  x <= load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z <= load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            m_chunk_map[origin] = LoadChunk(this, origin);
            count++;
        }
    }

    PrintDebug("Created %d chunks.\n", count);

    // Recalc and realize every chunk that we've loaded.
    // This leaves a ring of unrealized chunks around us, and that's okay.
    for     (int x = load_region.west();  x <= load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z <= load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);
            Chunk *pChunk = m_chunk_map[origin];
            pChunk->recalcExposures();
            pChunk->realizeLandscape();
        }
    }

    // Sanity check.
    for (auto iter : m_chunk_map) {
        Chunk *chunk = iter.second;
        if (!chunk->isLandcsapeRealized()) {
            assert(false);
        }
    }
}


// Game world destructor.
GameWorld::~GameWorld()
{
    sqlite3_close(m_database);
}


// Figure out where the camera will start.
// For now, smack in the center of the starting chunk,
// and one quarter of the way up.
MyVec4 GameWorld::getCameraStartPos() const
{
    GLfloat x = BLOCK_SCALE * CHUNK_WIDTH  * 0.5f;
    GLfloat y = BLOCK_SCALE * CHUNK_HEIGHT * 0.1f;
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
    GridCoord  new_location = WorldToGridCoord(m_camera_pos, NUDGE_NONE);

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

    // For any chunk at the "edge" of the load region, load it if we don't have it already.
    for     (int x = load_region.west();  x <= load_region.east();  x += CHUNK_WIDTH) {
        for (int z = load_region.south(); z <= load_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);

            if (NOT(draw_region.containsOrigin(origin))) {
                if (m_chunk_map.find(origin) == m_chunk_map.end()) {
                    Chunk *chunk = LoadChunk(this, origin);;
                    m_chunk_map[origin] = chunk;
                }
            }
        }
    }

    // Then, only recalc the chunks within the draw region.
    for     (int x = draw_region.west();  x <= draw_region.east();  x += CHUNK_WIDTH) {
        for (int z = draw_region.south(); z <= draw_region.north(); z += CHUNK_WIDTH) {
            ChunkOrigin origin(x, z);

            auto &chunk_iter = m_chunk_map.find(origin);
            assert(chunk_iter != m_chunk_map.end());

            // Finally, rebuild the landscape as needed.
            Chunk *chunk = chunk_iter->second;
            if (NOT(chunk->areExposuresCurrent())) {
                chunk->recalcExposures();
            }
            if (NOT(chunk->isLandcsapeRealized())) {
                chunk->realizeLandscape();
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

    for     (int x = region.west();  x <= region.east();  x += CHUNK_WIDTH) {
        for (int z = region.south(); z <= region.north(); z += CHUNK_WIDTH) {
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
        GridCoord coord  = m_hit_test_detail.getGlobalCoord();

        const auto &iter = m_chunk_map.find(origin);
        Chunk *pChunk = iter->second;

        assert(pChunk != nullptr);
        assert(pChunk->isCoordWithin(coord));

        pChunk->getBlockGlobal(coord)->setContent(CONTENT_AIR);
        pChunk->recalcExposures();
        pChunk->realizeLandscape();
    }
}

#pragma once

#include "stdafx.h"
#include "my_math.h"
#include "chunk_hit_test.h"
#include "chunk_loader.h"


struct sqlite3;
class  DrawState_PCT;
class  DrawState_PT;
class  EventStateMsg;
class  ChunkOrigin;


// Our game state.
class GameWorld
{
public:
    static GameWorld *Create();
    ~GameWorld();

    sqlite3 *getDatabase() { return m_database; }

    void setPaused(bool paused) { m_paused = paused; }
    bool isPaused() const { return m_paused; }

    void onGameTick(int elapsed_msec, const EventStateMsg &msg);

    int     getTimeMsecs() const { return m_time_msec; }
    GLfloat getTimeSecs()  const { return m_time_msec / 1000.0f; }

    GLfloat getCameraPitch() const { return m_camera_pitch; }
    GLfloat getCameraYaw()   const { return m_camera_yaw; }
    const MyVec4 &getCameraPos() const { return m_camera_pos; }

    const Chunk *getPlayersChunk() const;
    const Chunk *getRequiredChunk(const ChunkOrigin &origin) const;
    const Chunk *getOptionalChunk(const ChunkOrigin &origin) const;
    int getChunksInMemoryCount() const { return m_chunk_map.size(); }

    bool getHitTestSuccess() const { return m_hit_test_success; }
    const ChunkHitTestDetail &getHitTestDetail() const { return m_hit_test_detail; }
    const VertList_PT &getHitTestVertList() const { return m_hit_test_vert_list; }

    MyRay getCameraEyeRay() const;

    void resetCamera();
    void setCameraYaw(GLfloat val)   { m_camera_yaw = val; clampRotations(); }
    void setCameraPitch(GLfloat val) { m_camera_pitch = val; clampRotations(); }

    void deleteBlockInFrontOfUs();

private: 
    // Our constructor is private, and can only be called via "create".
    GameWorld(sqlite3 *database);

    // Disallow default ctor and copying.
    GameWorld() = delete;
    GameWorld(const GameWorld &that) = delete;
    void operator=(const GameWorld &that) = delete;

    // Every couple of seconds, cash in a worker thread.
    static const int WORKER_PACE_MSECS = 2000;

    MyVec4 getCameraStartPos() const;
    void updateWorld();
    void clampRotations();
    void calcHitTest();

    sqlite3 *m_database;

    bool m_paused;
    int  m_time_msec;
    
    GLfloat m_camera_pitch;
    GLfloat m_camera_yaw;
    MyVec4  m_camera_pos;

    GridCoord   m_current_grid_coord;
    ChunkOrigin m_current_chunk_origin;

    std::map<ChunkOrigin, Chunk *> m_chunk_map;
    bool m_hit_test_success;
    ChunkHitTestDetail m_hit_test_detail;
    VertList_PT m_hit_test_vert_list;
};

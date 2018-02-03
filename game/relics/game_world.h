#pragma once

#include "stdafx.h"

#include "chunk_io.h"
#include "hit_test_result.h"
#include "my_math.h"
#include "wavefront_object.h"


struct sqlite3;
class  ChunkOrigin;
class  DrawState_PCT;
class  DrawState_PT;
class  EventStateMsg;
class  Player;


// Our game state.
class GameWorld
{
public:
    GameWorld(const std::string &db_fname);
    ~GameWorld();

    const std::string &getDatabaseFilename() { return m_db_fname; }

    // TODO: Clean up events and make this constant again.
    Player &getPlayer() const { return *m_player; }

    void setPaused(bool paused) { m_paused = paused; }

    void onGameTick(int elapsed_msec, const EventStateMsg &msg);

    const Chunk *getRequiredChunk(const ChunkOrigin &origin) const;
    const Chunk *getOptionalChunk(const ChunkOrigin &origin) const;

    void setPlayerAtStart();
    void deleteBlockInFrontOfUs();

    // Getters.
    bool isPaused() const { return m_paused; }

    int getChunksInMemoryCount() const { return m_chunk_map.size(); }

    int     getTimeMsecs() const { return m_time_msec; }
    GLfloat getTimeSecs()  const { return m_time_msec / 1000.0f; }

    bool getHitTestSuccess() const { return m_hit_test_success; }
    const HitTestResult &getHitTestResult()   const { return m_hit_test_result; }
    const VertList_PT   &getHitTestVertList() const { return m_hit_test_vert_list; }

private:
    FORBID_DEFAULT_CTOR(GameWorld)
    FORBID_COPYING(GameWorld)
    FORBID_MOVING(GameWorld)

    // Every couple of seconds, cash in a worker thread.
    static const int WORKER_PACE_MSECS = 2000;

    void updateWorld();
    void calcHitTest();

    // Private data
    std::string m_db_fname;
    std::unique_ptr<Player> m_player;

    bool m_paused;
    int  m_time_msec;

    GlobalGrid  m_current_grid_coord;
    ChunkOrigin m_current_chunk_origin;

    std::map<ChunkOrigin, std::unique_ptr<Chunk>> m_chunk_map;

    bool m_hit_test_success;
    HitTestResult m_hit_test_result;
    VertList_PT m_hit_test_vert_list;
};

#pragma once

#include "stdafx.h"
#include "build_settings.h"
#include "build_stats.h"
#include "common_util.h"


// Game data, represented in a way that's "wxWidgets" friendly.
// It would be nice to unify any code here with the game's
// more OpenGL-oriented code, but we won't worry about that for now.

struct sqlite3;
struct sqlite3_stmt;


class WorldData
{
public:
    WorldData(const BuildSettings &settings);
    ~WorldData();

    BuildStats performDryRun() const;
    bool saveToDatabase(const std::string &db_fname);

    wxBitmap *getHeightmap() { return m_height_map; }

private:
    // Disallow the default ctor, copying, and moving.
    WorldData() = delete;
    WorldData(const WorldData &that) = delete;
    void operator=(const WorldData &that) = delete;
    WorldData(WorldData &&that) = delete;
    void operator=(WorldData &&that) = delete;

    // Private methods.
    bool actualSaveToDatabase(const std::string &fname, BuildStats *pOut_stats);
    bool initTables(sqlite3 *db);
    std::vector<BlockType> calcColumn(int world_x, int world_z, int dirt_height) const;
    int  writeBlocksForColumn(int world_x, int world_z, const std::vector<BlockType> &blocks,
                              sqlite3 *db, sqlite3_stmt *insert_stmt, BuildStats *pOut_stats);
    int  calcStoneHeightForColumn(int world_x, int world_z, int dirt_height) const;

    // Private data.
    BuildSettings m_build_settings;
    BuildStats    m_build_stats;

    std::string m_database_fname;
    wxBitmap *m_height_map;
};

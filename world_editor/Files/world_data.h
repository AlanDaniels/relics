#pragma once

#include "stdafx.h"


// Game data, represented in a way that's "wxWidgets" friendly.
// It would be nice to unify any code here with the game's
// more OpenGL-oriented code, but we won't worry about that for now.

class WorldData
{
public:
    WorldData(const std::string &heightmap_fname);
    ~WorldData();

    bool saveToDatabase(const std::string &db_fname);

    wxBitmap *getHeightmap() { return m_heightmap; }

private:
    // Disallow the default ctor, copying, and moving.
    WorldData() = delete;
    WorldData(const WorldData &that) = delete;
    void operator=(const WorldData &that) = delete;
    WorldData(WorldData &&that) = delete;
    void operator=(WorldData &&that) = delete;

    // Private methods.
    bool actualSaveToDatabase(const std::string &db_fname, int *pOut_blocks_written);

    // Private data.
    std::string m_heightmap_fname;
    std::string m_db_fname;

    wxBitmap *m_heightmap;
};
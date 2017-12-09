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

    void saveToDatabase(const std::string &db_fname);

    wxBitmap *getHeightmap() { return m_heightmap; }

private:
    // Disallow the default ctor.
    WorldData() = delete;

    std::string m_heightmap_fname;
    std::string m_db_fname;

    wxBitmap *m_heightmap;
};
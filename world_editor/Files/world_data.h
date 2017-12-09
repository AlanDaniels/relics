#pragma once

#include "stdafx.h"


// Game data, represented in a way that's "wxWidgets" friendly.
// It would be nice to unify any code here with the game's
// more OpenGL-oriented code, but we won't worry about that for now.

class WorldData
{
public:
    WorldData(const std::string &fname);
    ~WorldData();

    wxBitmap *getBitmap() { return m_bitmap; }

private:
    // Disallow the default ctor.
    WorldData() = delete;

    wxBitmap *trimBorders(wxBitmap *bitmap);

    std::string m_image_fname;
    wxBitmap *m_bitmap;
};
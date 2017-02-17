#pragma once

#include "stdafx.h"


// Just call this once, and load everything in one fell swoop.
bool LoadEditorResources();
void FreeEditorResources();

// And what gets loaded.
extern wxBitmap *toolbar_bitmap_wall;
extern wxBitmap *toolbar_bitmap_floor;
extern wxBitmap *toolbar_bitmap_entity;
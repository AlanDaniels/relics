#pragma once

#include "stdafx.h"
#include "sqlite3.h"


extern const char *RESOURCE_PATH;

bool LoadEditorResources();
bool FreeEditorResources();

wxBitmap *LoadPNGBitmap(const std::string &fname);

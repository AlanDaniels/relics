#pragma once

#include "stdafx.h"


// Let's keep these around for now, for when we add back a toolbar.
bool LoadEditorResources();
bool FreeEditorResources();

wxBitmap *LoadPNGBitmap(const std::string &fname);

#pragma once

#include "stdafx.h"
#include "sqlite3.h"


extern const char *RESOURCE_PATH;

bool LoadEditorResources();
bool FreeEditorResources();

wxBitmap *LoadPNGBitmap(const std::string &fname);

// Utility functions for SQLite.
// Meh. Maybe we could make this more OO later, but this is good for now.
sqlite3 *SQL_open(const std::string &fname);
bool SQL_exec(sqlite3 *db, const std::string &sql);
sqlite3_stmt *SQL_prepare(sqlite3 *db, const std::string &sql);
std::string SQL_code_to_str(int code);
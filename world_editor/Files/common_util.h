#pragma once

#include "stdafx.h"

/*
* NOTE: The only code that should ever go in here is truly
* common code, independent of whatever wxWidgets, OpenGL, or SFML
* thinks code should look like.
*
* Ideally we'd keep one copy of this code, but that messes with
* the builds, so for now we'll just copy and paste it since it
* doesn't change that often.
*/

// Debug printing.
void  PrintDebug(const char *format, ...);
void  PrintTheImpossible(const char *fname, int line_num, int value);

// Including the SQLite header causes conflicts. Not worth it.
struct sqlite3;
struct sqlite3_stmt;

// Utility functions for SQLite.
sqlite3 *SQL_open(const std::string &fname);
bool SQL_exec(sqlite3 *db, const std::string &sql);
sqlite3_stmt *SQL_prepare(sqlite3 *db, const std::string &sql);
std::string SQL_code_to_str(int code);
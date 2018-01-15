#pragma once

#include "stdafx.h"

/**
* Common data structures and utility methods.
*
* The ONLY code that should ever go in here is truly common
* code, independent of whatever wxWidgets, OpenGL, or SFML
* thinks code should look like.
*
* Ideally we'd keep one copy of this code, but that messes with
* the builds, so for now we'll just copy and paste it since it
* doesn't change that often.
*
* I found the string trimming functions here:
*     https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
*
* This version last updated on Jan 14th, 2018.
*/

enum class BlockType : unsigned char {
    AIR   = 0,
    DIRT  = 1,
    STONE = 2,
    COAL  = 3
};


// String trimming.
void LeftTrimInPlace(std::string *s);
void RightTrimInPlace(std::string *s);
void TrimInPlace(std::string *s);

// Debug printing.
std::string ReadableNumber(int val);
int GetMemoryUsage();

void PrintDebug(const std::string &msg);
void PrintTheImpossible(const std::string &fname, int line_num, int value);

// Including the SQLite header causes conflicts. Not worth it.
struct sqlite3;
struct sqlite3_stmt;

// Utility functions for SQLite.
sqlite3 *SQL_open(const std::string &fname);
bool SQL_exec(sqlite3 *db, const std::string &sql);
sqlite3_stmt *SQL_prepare(sqlite3 *db, const std::string &sql);
std::string SQL_code_to_str(int code);
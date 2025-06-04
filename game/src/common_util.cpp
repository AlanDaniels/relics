
#include "stdafx.h"
#include "common_util.h"

#include "format.h"
#include "sqlite3.h"
#include <psapi.h>

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
* This version last updated on Feb 3rd, 2018.
*/


// Left trim, in place.
void LeftTrimInPlace(std::string *s) {
    s->erase(s->begin(), std::find_if(s->begin(), s->end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// Right trim, in place.
void RightTrimInPlace(std::string *s) {
    s->erase(std::find_if(s->rbegin(), s->rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s->end());
}

// Trim both sides, in place.
void TrimInPlace(std::string *s) {
    LeftTrimInPlace(s);
    RightTrimInPlace(s);
}


// Turn a number into a nice readable string.
std::string ReadableNumber(int value) {
    const int GB = 1024 * 1024 * 1024;
    const int MB = 1024 * 1024;
    const int KB = 1024;

    if (value >= GB) {
        return fmt::format("{0:.1f}G", value / float(GB));
    }
    else if (value >= MB) {
        return fmt::format("{0:.1f}M", value / float(MB));
    }
    else if (value >= KB) {
        return fmt::format("{0:.1f}K", value / float(KB));
    }
    else {
        return fmt::format("{0}", value);
    }
}


// Find out how much memory the app is using.
int GetMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    memset(&pmc, 0, sizeof(pmc));
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);

    HANDLE hProc = GetCurrentProcess();

    GetProcessMemoryInfo(hProc, 
        reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc), 
        sizeof(PROCESS_MEMORY_COUNTERS_EX));
    int result = pmc.PrivateUsage;
    return result;
}


// Print a debug message, both to the Visual Studio debugger, and stdout.
// TODO: I'm not sure why, but using a "std::unique_ptr" to a new char array
// causes a crash here.
void PrintDebug(const std::string &msg)
{
    OutputDebugStringA(msg.c_str());
    printf(msg.c_str());
}


// Print when we hit an "impossible" case in a switch statement, and kill the app.
void PrintTheImpossible(const std::string &fname, int line_num, int value)
{
    PrintDebug(fmt::format(
        "IMPOSSIBLE VALUE! {0} (line {1}), value of {2}\n.",
        fname, line_num, value));
    assert(false);
}


// Round an integer up to the nearest multiple.
// We've had to roll our own here since negative numbers are tricky.
int RoundUpInt(int val, int mult)
{
    assert(mult > 0);

    int remainder = abs(val) % mult;
    if (remainder == 0) {
        return val;
    }

    if (val >= 0) {
        return val + mult - remainder;
    }
    else {
        return -(abs(val) - remainder);
    }
}


// Round an integer down to the nearest multiple.
// We've had to roll our own here since negative numbers are tricky.
int RoundDownInt(int val, int mult)
{
    assert(mult > 0);

    int remainder = abs(val) % mult;
    if (remainder == 0) {
        return val;
    }

    if (val >= 0) {
        return val - remainder;
    }
    else {
        return -(abs(val) + mult - remainder);
    }
}


// Utility function for opening a SQLite database.
sqlite3 *SQL_open(const std::string &fname)
{
    sqlite3 *db = nullptr;

    int ret_code = sqlite3_open(fname.c_str(), &db);
    if (ret_code == 0) {
        return db;
    }
    else {
        PrintDebug(fmt::format(
            "Could not open database:\n"
            "Error = {0}\n"
            "File  = {1}",
            sqlite3_errmsg(db), fname));

        sqlite3_close(db);
        return nullptr;
    }
}


// Utility function for closing a SQLite database.
bool SQL_close(sqlite3 *db)
{
    int ret_code = sqlite3_close(db);
    if (ret_code == SQLITE_OK) {
        return true;
    }
    else {
        PrintDebug(fmt::format(
            "Error closing SQLite database:\n"
            "Code  = {0}",
            SQL_code_to_str(ret_code)));
        return false;
    }
}


// Utility function for executing a SQLite statement all at once.
bool SQL_exec(sqlite3 *db, const std::string &sql)
{
    char *error_from_db = nullptr;

    int ret_code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_from_db);
    if (ret_code == SQLITE_OK) {
        return true;
    }
    else {
        PrintDebug(fmt::format(
            "Error running SQL:\n"
            "Code  = {0}"
            "Error = {1}\n"
            "SQL   = {2}",
            SQL_code_to_str(ret_code), error_from_db, sql));

        sqlite3_free(error_from_db);
        sqlite3_close(db);
        return false;
    }
}


// Utility function for preparing a SQLite statement.
sqlite3_stmt *SQL_prepare(sqlite3 *db, const std::string &sql)
{
    sqlite3_stmt* stmt = nullptr;

    int ret_code = sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr);
    if (ret_code == SQLITE_OK) {
        return stmt;
    }
    else {
        PrintDebug(fmt::format(
            "Error creating prepared statement:\n"
            "Code  = {0}\n"
            "SQL   = {1}",
            SQL_code_to_str(ret_code), sql));

        sqlite3_close(db);
        return nullptr;
    }
}


// Utility function for finalizing a prepared statement.
bool SQL_finalize(sqlite3 *db, sqlite3_stmt *stmt)
{
    int ret_code = sqlite3_finalize(stmt);
    if (ret_code == SQLITE_OK) {
        return true;
    }
    else {
        PrintDebug(fmt::format(
            "Error finalizing prepared statement:\n"
            "Code  = {0}\n",
            SQL_code_to_str(ret_code)));

        sqlite3_close(db);
        return false;
    }
}


// Utility code for turning a SQLite error code into a string.
std::string SQL_code_to_str(int code)
{
    switch (code) {
        // Everything went normally. Yay.
    case SQLITE_OK: return "SQLITE_OK";

        // Regular errors codes.
    case SQLITE_ERROR:      return "SQLITE_ERROR";
    case SQLITE_INTERNAL:   return "SQLITE_INTERNAL";
    case SQLITE_PERM:       return "SQLITE_PERM";
    case SQLITE_ABORT:      return "SQLITE_ABORT";
    case SQLITE_BUSY:       return "SQLITE_BUSY";
    case SQLITE_LOCKED:     return "SQLITE_LOCKED";
    case SQLITE_NOMEM:      return "SQLITE_NOMEM";
    case SQLITE_READONLY:   return "SQLITE_READONLY";
    case SQLITE_INTERRUPT:  return "SQLITE_INTERRUPT";
    case SQLITE_IOERR:      return "SQLITE_IOERR";
    case SQLITE_CORRUPT:    return "SQLITE_CORRUPT";
    case SQLITE_NOTFOUND:   return "SQLITE_NOTFOUND";
    case SQLITE_FULL:       return "SQLITE_FULL";
    case SQLITE_CANTOPEN:   return "SQLITE_CANTOPEN";
    case SQLITE_PROTOCOL:   return "SQLITE_PROTOCOL";
    case SQLITE_EMPTY:      return "SQLITE_EMPTY";
    case SQLITE_SCHEMA:     return "SQLITE_SCHEMA";
    case SQLITE_TOOBIG:     return "SQLITE_TOOBIG";
    case SQLITE_CONSTRAINT: return "SQLITE_CONSTRAINT";
    case SQLITE_MISMATCH:   return "SQLITE_MISMATCH";
    case SQLITE_MISUSE:     return "SQLITE_MISUSE";
    case SQLITE_NOLFS:      return "SQLITE_NOLFS";
    case SQLITE_AUTH:       return "SQLITE_AUTH";
    case SQLITE_FORMAT:     return "SQLITE_FORMAT";
    case SQLITE_RANGE:      return "SQLITE_RANGE";
    case SQLITE_NOTADB:     return "SQLITE_NOTADB";
    case SQLITE_NOTICE:     return "SQLITE_NOTICE";
    case SQLITE_WARNING:    return "SQLITE_WARNING";

        // These are normal return values from "sqlite3_step".
    case SQLITE_ROW:  return "SQLITE_ROW";
    case SQLITE_DONE: return "SQLITE_DONE";

        // Anything not covered so far is an extended error. Very unlikely.
    default: {
            return fmt::format("Extended error code {}", code);
        }
    }
}
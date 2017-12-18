
#include "stdafx.h"
#include "common_util.h"

#include "sqlite3.h"
#include <psapi.h>


// Turn a number into a nice readable string.
std::string ReadableNumber(int value) {
    char buffer[32];

    const int GB = 1024 * 1024 * 1024;
    const int MB = 1024 * 1024;
    const int KB = 1024;

    if (value >= GB) {
        sprintf(buffer, "%.1fG", value / float(GB));
    }
    else if (value >= MB) {
        sprintf(buffer, "%.1fM", value / float(MB));
    }
    else if (value >= KB) {
        sprintf(buffer, "%.1fK", value / float(KB));
    }
    else {
        sprintf(buffer, "%d", value);
    }

    return std::string(buffer);
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
void PrintDebug(const char *format, ...)
{
    char buffer[256];
    memset(buffer, '\0', sizeof(buffer));

    va_list arg_list;

    va_start(arg_list, format);
    vsprintf(buffer, format, arg_list);
    if (buffer[255] != '\0') {
        assert(buffer[255] == '\0');
    }
    va_end(arg_list);

    OutputDebugStringA(buffer);
    printf(buffer);
}


// Print when we hit an "impossible" case in a switch statement, and kill the app.
void PrintTheImpossible(const char *fname, int line_num, int value)
{
    PrintDebug(
        "IMPOSSIBLE VALUE! %s (line %d), value of %d\n.",
        fname, line_num, value);
    assert(false);
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
        PrintDebug(
            "Could not open database:\n"
            "Error = %s\n"
            "File  = %s",
            sqlite3_errmsg(db), fname.c_str());

        sqlite3_close(db);
        return nullptr;
    }
}

// Utility function for executing a SQLite statement.
bool SQL_exec(sqlite3 *db, const std::string &sql)
{
    char *error_from_db = nullptr;

    int ret_code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_from_db);
    if (ret_code == SQLITE_OK) {
        return true;
    }
    else {
        PrintDebug(
            "Error running SQL:\n"
            "Code  = %s"
            "Error = %s\n"
            "SQL   = %s",
            SQL_code_to_str(ret_code).c_str(), error_from_db, sql.c_str());

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
        PrintDebug(
            "Error creating prepared statement:\n"
            "Code  = %s\n"
            "SQL   = %s",
            SQL_code_to_str(ret_code).c_str(), sql.c_str());

        sqlite3_close(db);
        return nullptr;
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
            char msg[64];
            sprintf(msg, "Extended error code %d", code);
            return std::string(msg);
        }
    }
}
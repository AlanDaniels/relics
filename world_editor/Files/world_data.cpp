
#include "stdafx.h"
#include "world_data.h"
#include "common_util.h"
#include "sqlite3.h"


// The only allowed constructor.
WorldData::WorldData(const std::string &heightmap_fname) :
    m_heightmap_fname(heightmap_fname),
    m_db_fname("")
{
    if (!wxFileExists(heightmap_fname)) {
        char msg[64];
        sprintf(msg, "File %s does not exist!", heightmap_fname.c_str());
        assert(false);
    }

    wxBitmap *bitmap = new wxBitmap(heightmap_fname, wxBITMAP_TYPE_PNG);
    m_heightmap = bitmap;
}


// Destructor.
WorldData::~WorldData()
{
    m_heightmap->FreeResource();
    delete m_heightmap;
    m_heightmap = nullptr;
}



// Save our world to the database.
bool WorldData::saveToDatabase(const std::string &fname) {
    
    std::int64_t start_msec = wxGetLocalTimeMillis().GetValue();

    wxBeginBusyCursor();
    int blocks_written = 0;
    bool result = actualSaveToDatabase(fname, &blocks_written);
    wxEndBusyCursor();

    std::int64_t end_msec = wxGetLocalTimeMillis().GetValue();
    double elapsed = (end_msec - start_msec) / 1000.0;

    std::unique_ptr<char[]> buffer(new char[256]);
    sprintf(buffer.get(),
        "Blocks written: %d\n"
        "Elapsed time: %0.3f seconds",
        blocks_written, elapsed);
    wxLogMessage(buffer.get());

    return result;
}


// This does the real work.
// We call this from a wrapper function so the wrapper
// can change the cursor and time how long it took.
bool WorldData::actualSaveToDatabase(const std::string &fname, int *pOut_blocks_written)
{
    // For our scratchpad, allocate it on the heap.
    std::unique_ptr<char[]> buffer(new char[256]);

    // First, make sure we can sample the heightmap.
    wxNativePixelData pixels(*m_heightmap);
    if (!pixels) {
        return false;
    }

    // Not sure if this is expensive, so only creating the one here.
    wxNativePixelData::Iterator pixel_iter;

    // Create our database.
    char *error_from_db = nullptr;
    int ret_code;

    bool success;

    sqlite3 *db = SQL_open(fname);
    if (db == nullptr) {
        return false;
    }

    // Create our main "blocks" table.
    const char *create_blocks_sql =
        "CREATE TABLE blocks ("
        "x INTEGER, "
        "y INTEGER, "
        "z INTEGER, "
        "block_type VARCHAR(10) NOT NULL, "
        "PRIMARY KEY (x, y, z))";

    success = SQL_exec(db, create_blocks_sql);
    if (!success) {
        return false;
    }

    // Begin our transaction.
    success = SQL_exec(db, "BEGIN TRANSACTION");
    if (!success) {
        return false;
    }

    // We'll be calling "insert" on the blocks table a
    // few million times, so use a prepared statement.
    const char *insert_sql =
        "INSERT INTO blocks (x, y, z, block_type) "
        "VALUES (?1, ?2, ?3, 'dirt')";

    sqlite3_stmt* insert_stmt = SQL_prepare(db, insert_sql);
    if (insert_stmt == nullptr) {
        return false;
    }

    int blocks_written = 0;

    // For each spot on our heightmap, write the value of 'dirt_top' where the dirt world
    // actually start. Writing a value of 'dirt' for each individual block would take forever.
    int width  = m_heightmap->GetWidth();
    int height = m_heightmap->GetHeight();
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            pixel_iter.MoveTo(*m_heightmap, x, y);

            unsigned char red   = pixel_iter.Red();
            unsigned char green = pixel_iter.Green();
            unsigned char blue  = pixel_iter.Blue();

            int dirt_top = (red + green + blue) / 3;
            if (dirt_top > 0) {
                int true_x = x - (width  / 2);
                int true_y = y - (height / 2);

                sqlite3_bind_int(insert_stmt, 1, x);
                sqlite3_bind_int(insert_stmt, 2, y);
                sqlite3_bind_int(insert_stmt, 3, dirt_top);

                ret_code = sqlite3_step(insert_stmt);
                if (ret_code != SQLITE_DONE) {
                    sprintf(buffer.get(),
                        "Step failed in \"%s\", error %s",
                        fname.c_str(), error_from_db);
                    wxLogMessage(buffer.get());

                    sqlite3_free(error_from_db);
                    sqlite3_close(db);
                    return false;
                }

                sqlite3_reset(insert_stmt);
                blocks_written++;
            }
        }   
    }

    // Complete our transaction.
    success = SQL_exec(db, "COMMIT TRANSACTION");
    if (!success) {
        return false;
    }

    sqlite3_finalize(insert_stmt);
 
    // All done.
    sqlite3_close(db);
    *pOut_blocks_written = blocks_written;
    return true;
}
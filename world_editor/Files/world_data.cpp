
#include "stdafx.h"
#include "world_data.h"

#include "common_util.h"
#include "simplex_noise.h"

#include "sqlite3.h"



// Given our landscape top, calc where the stone top.
// Scale it, lower it, then add some noise. Use "-1" to mean no value.
int CalcStoneHeight(int x, int z, int dirt_height)
{
    // We'll want to play around with these numbers later.
    const int    LOWER_BY = 5;
    const double SCALE_BY = 0.65;
    const int    NOISE_BUMP = 10;

    int result = (dirt_height * SCALE_BY) - LOWER_BY;

    double noise_val = simplex_noise_2(x, z);
    noise_val -= 0.5;
    result += (noise_val * NOISE_BUMP);

    if (result >= dirt_height) {
        result = dirt_height - 1;
    }

    if (result < -1) {
        result = -1;
    }

    return result;
}


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
    bool success = actualSaveToDatabase(fname, &blocks_written);
    wxEndBusyCursor();

    std::int64_t end_msec = wxGetLocalTimeMillis().GetValue();
    double elapsed = (end_msec - start_msec) / 1000.0;

    std::unique_ptr<char[]> buffer(new char[256]);
    if (success) {
        sprintf(buffer.get(),
            "Blocks written: %d\n"
            "Elapsed time: %0.1f seconds",
            blocks_written, elapsed);
    }
    else {
        sprintf(buffer.get(), "SQL Error! Check your logs.");
    }

    wxLogMessage(buffer.get());

    return success;
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
    int ret_code;
    bool success;

    sqlite3 *db = SQL_open(fname);
    if (db == nullptr) {
        return false;
    }

    // Create our main "blocks" table.
    success = SQL_exec(db,
        "CREATE TABLE blocks ("
        "x INTEGER, "
        "y INTEGER, "
        "z INTEGER, "
        "block_type VARCHAR(10) NOT NULL, "
        "PRIMARY KEY (x, y, z))");
    if (!success) {
        return false;
    }

    success = SQL_exec(db, "CREATE INDEX idx_blocks_x ON blocks (x ASC)");
    if (!success) {
        return false;
    }

    success = SQL_exec(db, "CREATE INDEX idx_blocks_y ON blocks (y ASC)");
    if (!success) {
        return false;
    }

    success = SQL_exec(db, "CREATE INDEX idx_blocks_z ON blocks (z ASC)");
    if (!success) {
        return false;
    }

    // Begin our transaction.
    success = SQL_exec(db, "BEGIN TRANSACTION");
    if (!success) {
        return false;
    }

    // Create our "insert blocks" statement.
    sqlite3_stmt *insert_stmt = SQL_prepare(db,
        "INSERT INTO blocks (x, y, z, block_type) "
        "VALUES (?1, ?2, ?3, ?4)");
    if (insert_stmt == nullptr) {
        return false;
    }

    int blocks_written = 0;

    // SQLite string binding is a bit strange.
    const char *dirt_top_str = "dirt_top";
    const int   dirt_top_len = strlen(dirt_top_str);

    const char *stone_top_str = "stone_top";
    const int   stone_top_len = strlen(stone_top_str);

    // For each spot on our heightmap, write the value of 'dirt_top' where the dirt world
    // actually start. Writing a value of 'dirt' for each individual block would take forever.
    int height_map_width  = m_heightmap->GetWidth();
    int height_map_height = m_heightmap->GetHeight();
    for     (int x = 0; x < height_map_width;  x++) {
        for (int y = 0; y < height_map_height; y++) {
            pixel_iter.MoveTo(*m_heightmap, x, y);

            int red   = pixel_iter.Red();
            int green = pixel_iter.Green();
            int blue  = pixel_iter.Blue();

            int landscape_x =  x - (height_map_width  / 2);
            int landscape_z = -y + (height_map_height / 2);

            // For the dirt top, take half of our height map color.
            // Subtract one so that we don't have a two-block falloff at the edges.
            int dirt_height = (red + green + blue) / 3;
            dirt_height = (dirt_height / 2) - 1;

            if (dirt_height >= 0) {
                sqlite3_bind_int (insert_stmt, 1, landscape_x);
                sqlite3_bind_int (insert_stmt, 2, dirt_height);
                sqlite3_bind_int (insert_stmt, 3, landscape_z);
                sqlite3_bind_text(insert_stmt, 4, dirt_top_str, dirt_top_len, SQLITE_STATIC);

                ret_code = sqlite3_step(insert_stmt);
                if (ret_code != SQLITE_DONE) {
                    sprintf(buffer.get(),
                        "Insert failed, code = %s, error = %s",
                        SQL_code_to_str(ret_code).c_str(), 
                        sqlite3_errmsg(db));
                    wxLogMessage(buffer.get());

                    sqlite3_close(db);
                    return false;
                }

                sqlite3_reset(insert_stmt);
                blocks_written++;

                int stone_height = CalcStoneHeight(landscape_x, landscape_z, dirt_height);
                if (stone_height >= 0) {
                    sqlite3_bind_int (insert_stmt, 1, landscape_x);
                    sqlite3_bind_int (insert_stmt, 2, stone_height);
                    sqlite3_bind_int (insert_stmt, 3, landscape_z);
                    sqlite3_bind_text(insert_stmt, 4, stone_top_str, stone_top_len, SQLITE_STATIC);

                    ret_code = sqlite3_step(insert_stmt);
                    if (ret_code != SQLITE_DONE) {
                        sprintf(buffer.get(),
                            "Insert failed, code = %s, error = %s",
                            SQL_code_to_str(ret_code).c_str(),
                            sqlite3_errmsg(db));
                        wxLogMessage(buffer.get());

                        sqlite3_close(db);
                        return false;
                    }

                    sqlite3_reset(insert_stmt);
                    blocks_written++;
                }
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

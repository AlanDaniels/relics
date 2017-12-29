
#include "stdafx.h"
#include "world_data.h"

#include "common_util.h"
#include "format.h"
#include "simplex_noise.h"
#include "sqlite3.h"

#include <boost/filesystem.hpp>


// The only allowed constructor.
WorldData::WorldData(const BuildSettings &settings) :
    m_build_settings(settings),
    m_database_fname("")
{
    const std::string &fname = m_build_settings.getHeightMapFilename();

    if (!boost::filesystem::is_regular_file(fname)) {
        std::string msg = fmt::format("File '{}' does not exist.", fname);
        wxLogMessage(msg.c_str());
        assert(false);
    }

    wxBitmap *bitmap = new wxBitmap(fname, wxBITMAP_TYPE_PNG);
    m_height_map = bitmap;
}


// Destructor.
WorldData::~WorldData()
{
    m_height_map->FreeResource();
    delete m_height_map;
    m_height_map = nullptr;
}


// Peform a dry run, to see what our build stats would be,
// before performing all the I/O needed to write out the database.
BuildStats WorldData::performDryRun() const
{
    BuildStats stats;

    wxNativePixelData pixels(*m_height_map);
    if (!pixels) {
        return stats;
    }

    wxNativePixelData::Iterator pixel_iter;

    // Calc each column and write it to the database.
    int hmap_width  = m_height_map->GetWidth();
    int hmap_height = m_height_map->GetHeight();
    for (int x = 0; x < hmap_width; x++) {
        for (int y = 0; y < hmap_height; y++) {

            pixel_iter.MoveTo(*m_height_map, x, y);

            int red   = pixel_iter.Red();
            int green = pixel_iter.Green();
            int blue  = pixel_iter.Blue();

            int world_x =  x - (hmap_width  / 2);
            int world_z = -y + (hmap_height / 2);

            // For the dirt top, take half of our height map color.
            // Subtract one so that we don't have a two-block falloff at the edges.
            int dirt_height = (red + green + blue) / 3;
            dirt_height = (dirt_height / 2) - 1;

            if (dirt_height >= 0) {
                std::vector<BlockType> blocks = calcColumn(world_x, world_z, dirt_height);
                for (const BlockType & block : blocks) {
                    stats.add(block);
                }
            }
        }
    }

    return std::move(stats);
}



// Save our world to the database.
bool WorldData::saveToDatabase(const std::string &fname) {

    std::int64_t start_msec = wxGetLocalTimeMillis().GetValue();

    wxBeginBusyCursor();
    BuildStats stats;
    bool success = actualSaveToDatabase(fname, &stats);
    wxEndBusyCursor();

    std::int64_t end_msec = wxGetLocalTimeMillis().GetValue();
    double elapsed = (end_msec - start_msec) / 1000.0;

    std::string msg;
    if (success) {
        m_build_stats = stats;
        std::string stats_str = stats.toString();
        wxLogMessage(stats_str.c_str());
        return true;
    }
    else {
        wxLogMessage("SQL Error! Check your logs.");
        return false;
    }
}


// This does the real work.
// We call this from a wrapper function so the wrapper
// can change the cursor and time how long it took.
bool WorldData::actualSaveToDatabase(const std::string &fname, BuildStats *pOut_stats)
{
    // First, make sure we can sample the heightmap.
    wxNativePixelData pixels(*m_height_map);
    if (!pixels) {
        return false;
    }

    // I'm not sure if this is expensive, so I'm only creating the one here.
    wxNativePixelData::Iterator pixel_iter;

    // Create our database.
    bool success;

    sqlite3 *db = SQL_open(fname);
    if (db == nullptr) {
        return false;
    }

    if (!initTables(db)) {
        sqlite3_close(db);
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

    // Got this far? Congrats, we'll actually be writing data.
    BuildStats stats;

    // Calc each column and write it to the database.
    int hmap_width  = m_height_map->GetWidth();
    int hmap_height = m_height_map->GetHeight();
    for     (int x = 0; x < hmap_width;  x++) {
        for (int y = 0; y < hmap_height; y++) {

            pixel_iter.MoveTo(*m_height_map, x, y);

            int red   = pixel_iter.Red();
            int green = pixel_iter.Green();
            int blue  = pixel_iter.Blue();

            int world_x =  x - (hmap_width  / 2);
            int world_z = -y + (hmap_height / 2);

            // For the dirt top, take half of our height map color.
            // Subtract one so that we don't have a two-block falloff at the edges.
            int dirt_height = (red + green + blue) / 3;
            dirt_height = (dirt_height / 2) - 1;

            if (dirt_height >= 0) {
                std::vector<BlockType> blocks = calcColumn(world_x, world_z, dirt_height);
                writeBlocksForColumn(world_x, world_z, blocks, db, insert_stmt, &stats);
            }
        }   
    }

    // All done.
    success = SQL_exec(db, "COMMIT TRANSACTION");
    if (!success) {
        return false;
    }

    sqlite3_finalize(insert_stmt);
    sqlite3_close(db);

    *pOut_stats = stats;
    return true;
}


// Do the initial setup for the database.
// Clear out any old tables, and build new ones.
bool WorldData::initTables(sqlite3 *db)
{
    bool success;

    success = SQL_exec(db, "DROP TABLE IF EXISTS blocks");
    if (!success) {
        return false;
    }

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

    return true;
}


// Given our landscape top, calc where the stone top.
int WorldData::calcStoneHeightForColumn(int world_x, int world_z, int dirt_height) const
{
    double percent      = m_build_settings.getStonePercent();
    double subtracted   = m_build_settings.getStoneSubtracted();
    double displacement = m_build_settings.getStoneDisplacement();
    double noise_scale  = m_build_settings.getStoneNoiseScale();

    // First, scale the stone, then lower it.
    int result = (dirt_height * percent) - subtracted;

    // Scale our noise outward.
    double noise_x   = world_x / noise_scale;
    double noise_z   = world_z / noise_scale;
    double noise_val = simplex_noise_2(noise_x, noise_z) - 0.5;

    // Displace the result by our noise value.
    result += (noise_val * displacement);

    // Return -1 to mean there's no stone at all.
    if (result < -1) {
        result = -1;
    }

    return result;
}


// Calculate the world blocks for a particular column.
// Profile this later, since it might be a bottleneck.
std::vector<BlockType> WorldData::calcColumn(int world_x, int world_z, int dirt_height) const
{
    double noise_scale  = m_build_settings.getStoneNoiseScale();
    double coal_density = m_build_settings.getCoalDensity() / 100.0f;

    // Given our dirt height, calc how tall the stone could be.
    int stone_height = calcStoneHeightForColumn(world_x, world_z, dirt_height);

    // In rare cases, the stone could stick up *out* of the dirt.
    int ceiling = (dirt_height > stone_height) ? dirt_height : stone_height;

    // Build a vector for the Y-values, and fill it all with dirt.
    std::vector<BlockType> blocks;
    for (int y = 0; y <= ceiling; y++) {
        blocks.emplace_back(BlockType::DIRT);
    }

    // Then, replace all the stone blocks for that height.
    for (int y = 0; y <= stone_height; y++) {
        blocks[y] = BlockType::STONE;
    }

    // Throughout the stone, figure out where the coal would go.
    for (int y = 0; y <= stone_height; y++) {

        double noise_x = world_x / noise_scale;
        double noise_z = world_z / noise_scale;
        double noise_y = y / noise_scale;
        double noise_val = simplex_noise_3(noise_x, noise_y, noise_z);

        if (noise_val < coal_density) {
            blocks[y] = BlockType::COAL;
        }
    }

    return std::move(blocks);
}



// Write all the world blocks for a particular column.
// For SQLite string binding. Just hard-code the string lengthts.
// For each spot on our heightmap, write the value of 'dirt_top' where the dirt world
// actually start. Writing a value of 'dirt' for each individual block would take forever.
bool WorldData::writeBlocksForColumn(
    int world_x, int world_z, const std::vector<BlockType> &blocks,
    sqlite3 *db, sqlite3_stmt *insert_stmt, BuildStats *pOut_stats)
{
    int ret_code;

    // Calc the tops of the dirt and stone.
    int dirt_top  = -1;
    int stone_top = -1;
    for (unsigned int i = 0; i < blocks.size(); i++) {
        if (blocks[i] == BlockType::DIRT) {
            dirt_top = i;
        }
        else if (blocks[i] == BlockType::STONE) {
            stone_top = i;
        }
    }

    // Write the dirt top.
    sqlite3_bind_int(insert_stmt, 1, world_x);
    sqlite3_bind_int(insert_stmt, 2, dirt_top);
    sqlite3_bind_int(insert_stmt, 3, world_z);
    sqlite3_bind_text(insert_stmt, 4, "dirt_top", 8, SQLITE_STATIC);

    ret_code = sqlite3_step(insert_stmt);
    if (ret_code != SQLITE_DONE) {
        std::string msg = fmt::format(
            "Insert failed, code = {0}, error = {1}",
            SQL_code_to_str(ret_code),
            sqlite3_errmsg(db));
        wxLogMessage(msg.c_str());
        return false;
    }

    // Write the stone top (if there is one).
    if (stone_top >= 0) {
        sqlite3_reset(insert_stmt);

        sqlite3_bind_int(insert_stmt, 1, world_x);
        sqlite3_bind_int(insert_stmt, 2, stone_top);
        sqlite3_bind_int(insert_stmt, 3, world_z);
        sqlite3_bind_text(insert_stmt, 4, "stone_top", 9, SQLITE_STATIC);

        ret_code = sqlite3_step(insert_stmt);
        if (ret_code != SQLITE_DONE) {
            std::string msg = fmt::format(
                "Insert failed, code = {0}, error = {1}",
                SQL_code_to_str(ret_code),
                sqlite3_errmsg(db));
            wxLogMessage(msg.c_str());
            return false;
        }

        sqlite3_reset(insert_stmt);
    }

    // Then, write each individual coal block. There shouldn't be too many of these.
    for (int y = 0; y < stone_top; y++) {
        if (blocks[y] == BlockType::COAL) {
            sqlite3_reset(insert_stmt);

            sqlite3_bind_int(insert_stmt, 1, world_x);
            sqlite3_bind_int(insert_stmt, 2, y);
            sqlite3_bind_int(insert_stmt, 3, world_z);
            sqlite3_bind_text(insert_stmt, 4, "coal", 4, SQLITE_STATIC);

            ret_code = sqlite3_step(insert_stmt);
            if (ret_code != SQLITE_DONE) {
                std::string msg = fmt::format(
                    "Insert failed, code = {0}, error = {1}",
                    SQL_code_to_str(ret_code),
                    sqlite3_errmsg(db));
                wxLogMessage(msg.c_str());
                return false;
            }

            sqlite3_reset(insert_stmt);

        }
    }

    // All done. Update our stats.
    for (unsigned int y = 0; y < blocks.size(); y++) {
        pOut_stats->add(blocks[y]);
    }

    return true;
}

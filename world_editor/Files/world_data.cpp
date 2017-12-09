
#include "world_data.h"
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


// Save to file.
void WorldData::saveToDatabase(const std::string &db_name)
{
    // TODO: CONTINUE HERE.
}

#include "world_data.h"


// The only allowed constructor.
// TODO: Use boost to ensure this file exists.
WorldData::WorldData(const std::string &fname) :
    m_image_fname(fname)
{

    if (!wxFileExists(fname)) {
        char msg[64];
        sprintf(msg, "File %s does not exist!", fname.c_str());
        assert(false);
    }

    wxBitmap *bitmap = new wxBitmap(fname, wxBITMAP_TYPE_PNG);
    m_bitmap = bitmap;
}


// Destructor.
WorldData::~WorldData()
{
    delete m_bitmap;
    m_bitmap = nullptr;
}
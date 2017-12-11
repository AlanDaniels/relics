
#include "stdafx.h"
#include "util.h"


const char *RESOURCE_PATH = "D:\\relics\\resources";


// These don't do anything now, but keeping them
// around for when we add back a toolbar later.
bool LoadEditorResources() { return true; }
bool FreeEditorResources() { return true; }


wxBitmap *LoadPNGBitmap(const std::string &fname)
{
    std::ostringstream temp1;
    temp1 << RESOURCE_PATH << "\\" << fname;
    std::string full_name = temp1.str();

    if (!wxFileExists(full_name)) {
        std::ostringstream temp2;
        temp2 << "Missing resource: " << full_name;
        std::string error_msg = temp2.str();

        wxMessageBox(error_msg, "Resource Error", wxOK | wxICON_ERROR);
        return nullptr;
    }

    wxBitmap *result = new wxBitmap(full_name, wxBITMAP_TYPE_PNG);
    return result;
}

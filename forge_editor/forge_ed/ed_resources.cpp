
#include "stdafx.h"
#include "ed_resources.h"


// I'm keeping these around for when we add back a toolbar later.
bool LoadEditorResources() { return true; }
bool FreeEditorResources() { return true; }


// Constants.
const std::string RESOURCE_PATH = "C:\\forge_ahead\\forge_resources\\editor";


// After all these years, I still believe C++ is stupid on how it handles strings.
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

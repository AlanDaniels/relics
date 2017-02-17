
#include "stdafx.h"
#include "ed_resources.h"


// Our declared stuff.
wxBitmap *toolbar_bitmap_wall;
wxBitmap *toolbar_bitmap_floor;
wxBitmap *toolbar_bitmap_entity;


// Constants.
const std::string RESOURCE_PATH = "D:\\Projects\\forge_resources\\editor";


// After all these years, I still thing C++ is fucking stupid on how it handles strings.
wxBitmap *LoadPNGBitmap(const char *fname)
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


// Call this once, to load all the bitmaps and such the editor needs.
bool LoadEditorResources()
{
    toolbar_bitmap_wall = LoadPNGBitmap("wall.png");
    if (toolbar_bitmap_wall == nullptr) {
        return false;
    }

    toolbar_bitmap_floor = LoadPNGBitmap("floor.png");
    if (toolbar_bitmap_floor == nullptr) {
        return false;
    }

    toolbar_bitmap_entity = LoadPNGBitmap("entity.png");
    if (toolbar_bitmap_entity == nullptr) {
        return false;
    }

    return true;
}


// Delete everything, if only to avoid false positives on memory leaks.
void FreeEditorResources()
{
    delete toolbar_bitmap_wall;
    delete toolbar_bitmap_floor;
    delete toolbar_bitmap_entity;

    toolbar_bitmap_wall   = nullptr;
    toolbar_bitmap_floor  = nullptr;
    toolbar_bitmap_entity = nullptr;

}
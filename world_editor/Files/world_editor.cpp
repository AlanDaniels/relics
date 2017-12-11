
#include "stdafx.h"
#include "world_editor.h"

#include "my_canvas.h"
#include "util.h"
#include "world_data.h"


// Init the app.
bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    if (!LoadEditorResources()) {
        return false;
    }

    MyMainFrame *frame = new MyMainFrame();
    frame->Show(true);
    return true;
}


// Free up anything we've allocated.
int MyApp::OnExit()
{
    FreeEditorResources();
    return 0;
}


// Main Frame ctor.
MyMainFrame::MyMainFrame() :
    wxFrame(NULL, wxID_ANY, "Relics World Editor"),
    m_world_data(nullptr),
    m_canvas(nullptr)
{
    myCreateMenuBar();

    // A flex grid sizer, one row, one column, five pixels worth of border.
    wxFlexGridSizer *grid_sizer = new wxFlexGridSizer(1, 1, 5, 5);
    grid_sizer->AddGrowableRow(0);
    grid_sizer->AddGrowableCol(0);

    // Add our one canvas, using a sizer so it fills up the entire frame.
    m_canvas = new MyCanvas(this);
    grid_sizer->Add(m_canvas, 1, wxEXPAND | wxALL);

    // Play around with this until you like the results.
    SetSizer(grid_sizer);

    // Create the default status bar.
    CreateStatusBar();
    SetStatusText("Welcome to the Relics World Editor!");

    // Figure out our desired layout from the screen size.
    int screen_width  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int screen_height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    int border = 200;
    SetSize(border, border, screen_width - (border * 2), screen_height - (border * 2));
}


// Main Frame dtor.
MyMainFrame::~MyMainFrame()
{
    FreeEditorResources();
    delete m_world_data;
    m_world_data = nullptr;
}


// Create our menu bar.
void MyMainFrame::myCreateMenuBar()
{
    wxMenu *menu_file = new wxMenu;
    menu_file->Append(ID_MENU_NEW,  "&New\tCtrl-N", "Create a new world");
    menu_file->Append(ID_MENU_OPEN, "&Open\tCtrl-O", "Open an existing world");
    menu_file->Append(ID_MENU_SAVE, "&Save\tCtrl-S", "Save the current world");
    menu_file->Append(ID_MENU_SAVE_AS, "&Save As...", "Save under a new name");
    menu_file->AppendSeparator();
    menu_file->Append(wxID_EXIT);

    wxMenu *menu_edit = new wxMenu;
    menu_edit->Append(ID_MENU_CHANGE_IMAGE, "Change Image...", "Change the base image");

    wxMenu *menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menu_file, "&File");
    menuBar->Append(menu_edit, "&Edit");
    menuBar->Append(menu_help, "&Help");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuNew,    this, ID_MENU_NEW);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuOpen,   this, ID_MENU_OPEN);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuSave,   this, ID_MENU_SAVE);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuSaveAs, this, ID_MENU_SAVE_AS);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuExit,   this, wxID_EXIT);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuChangeImage, this, ID_MENU_CHANGE_IMAGE);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuAbout, this, wxID_ABOUT);
}


// Close out any old game file, and load an image for a new one.
void MyMainFrame::onMenuNew(wxCommandEvent &evt)
{
    if (m_world_data != nullptr) {
        delete m_world_data;
        m_world_data = nullptr;
    }

    wxString dlg_result = wxFileSelector(
        "Choose a base heightmap to start with...",
        RESOURCE_PATH,
        "*.png",
        ".png",
        "PNG Files (*.png)|*.png",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST,
        this);
    if (dlg_result.IsEmpty()) {
        return;
    }

    std::string fname(dlg_result.c_str());
    m_world_data = new WorldData(fname);
    m_canvas->repaintCanvas();

    char msg[128];
    sprintf(msg, "File: %s", fname.c_str());
    SetStatusText(msg);
}


void MyMainFrame::onMenuOpen(wxCommandEvent &evt)
{
    wxLogMessage("Hello world from wxWidgets!");
}


void MyMainFrame::onMenuSave(wxCommandEvent &evt)
{
    if (m_world_data == nullptr) {
        wxLogMessage("Nothing to save!");
        return;
    }

    wxString dlg_result = wxFileSelector(
        "Choose a World file name",
        RESOURCE_PATH,
        "",
        "*.world",
        "World Files (*.world)|*.world",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT,
        this);
    if (dlg_result.IsEmpty()) {
        return;
    }

    std::string fname(dlg_result.c_str());
    m_world_data->saveToDatabase(fname);
}


void MyMainFrame::onMenuSaveAs(wxCommandEvent &evt)
{
    wxLogMessage("Hello world from wxWidgets!");
}


void MyMainFrame::onMenuChangeImage(wxCommandEvent &evt)
{
    wxLogMessage("Hello world from wxWidgets!");
}


void MyMainFrame::onMenuExit(wxCommandEvent &evt)
{
    Close(true);
}


void MyMainFrame::onMenuAbout(wxCommandEvent &evt)
{
    wxMessageBox(
        "This is the level editor for Relics",
        "About Relics Ed", wxOK | wxICON_INFORMATION);
}


// Here's the wxWidgets equivalent of "main".
// Can't say I'm a big fan of the macro cleverness here, but so be it.
wxIMPLEMENT_APP(MyApp);

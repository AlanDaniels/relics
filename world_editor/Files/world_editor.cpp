
#include "stdafx.h"
#include "world_editor.h"

#include "my_canvas.h"
#include "ed_resources.h"


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
    m_canvas(nullptr)
{
    myCreateMenuBar();

    // A flex grid sizer, one row, three cols, five pixels worth of border.
    wxFlexGridSizer *grid_sizer = new wxFlexGridSizer(3, 5, 5, 5);
    grid_sizer->AddGrowableRow(0);
    grid_sizer->AddGrowableCol(1);

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
    int border = 100;
    SetSize(border, border, screen_width - (border * 2), screen_height - (border * 2));
}


// Main Frame dtor.
MyMainFrame::~MyMainFrame()
{
    FreeEditorResources();
}


// Create our menu bar.
void MyMainFrame::myCreateMenuBar()
{
    wxMenu *menu_file = new wxMenu;
    menu_file->Append(ID_MENU_FILE_NEW, "&New\tCtrl-N", "Create a new world map");
    menu_file->AppendSeparator();
    menu_file->Append(wxID_EXIT);

    wxMenu *menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menu_file, "&File");
    menuBar->Append(menu_help, "&Help");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuNew,   this, ID_MENU_FILE_NEW);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuExit,  this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuAbout, this, wxID_ABOUT);
}



void MyMainFrame::onMenuNew(wxCommandEvent &evt)
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

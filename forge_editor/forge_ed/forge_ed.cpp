
#include "stdafx.h"
#include "forge_ed.h"

#include "my_canvas.h"
#include "ed_resources.h"
#include "game_world.h"


// Settings ctor.
MySettings::MySettings() :
    m_tile_mode(TILE_MODE_WALL),
    m_draw_mode(DRAW_MODE_BOX)
{
}


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


// Main Frame ctor.
MyMainFrame::MyMainFrame() :
    wxFrame(NULL, wxID_ANY, "Relics Editor"),
    m_canvas(nullptr),
    m_game_world(nullptr)
{
    m_game_world = new GameLevel;

    myCreateMenuBar();
    myCreateToolBar();

    // A flex grid sizer, one row, three cols, five pixels worth of border.
    wxFlexGridSizer *grid_sizer = new wxFlexGridSizer(3, 5, 5, 5);
    grid_sizer->AddGrowableRow(0);
    grid_sizer->AddGrowableCol(1);

    // Add our list of levels.
    m_level_list = new wxListBox(this, ID_LEVEL_LIST);
    m_level_list->SetMinSize(wxSize(200, wxEXPAND));
    grid_sizer->Add(m_level_list, 1, wxEXPAND | wxALL);

    // Add our one canvas, using a sizer so it fills up the entire frame.
    m_canvas = new MyCanvas(this, m_game_world);
    grid_sizer->Add(m_canvas, 1, wxEXPAND | wxALL);

    // Add our list of entities.
    m_entity_list = new wxListBox(this, ID_ENTITY_LIST);
    m_entity_list->SetMinSize(wxSize(200, wxEXPAND));
    grid_sizer->Add(m_entity_list, 1, wxEXPAND | wxALL);

    // TODO: Keep playing around with this until you like the results.
    SetSizer(grid_sizer);

    // Create the default status bar.
    CreateStatusBar();
    SetStatusText("Welcome to Relics Editor, you Game Devloper, you!");

    // Figure out our desired layout from the screen size.
    int screen_width  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int screen_height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    int border = 100;
    SetSize(border, border, screen_width - (border * 2), screen_height - (border * 2));
}


// Main Frame dtor.
MyMainFrame::~MyMainFrame()
{
    delete m_game_world;
    m_game_world = nullptr;

    FreeEditorResources();
}


// Create our menu bar.
void MyMainFrame::myCreateMenuBar()
{
    wxMenu *menu_file = new wxMenu;
    menu_file->Append(ID_MENU_FILE_NEW, "&New\tCtrl-N", "Create a new world map");
    menu_file->AppendSeparator();
    menu_file->Append(wxID_EXIT);

    // TODO: CONTINUE HERE. What are better names for these?

    wxMenu *menu_mode = new wxMenu;
    menu_mode->AppendRadioItem(ID_MENU_TILEMODE_WALL,   "Draw With &Walls");
    menu_mode->AppendRadioItem(ID_MENU_TILEMODE_FLOOR,  "Draw With &Floors");
    menu_mode->AppendRadioItem(ID_MENU_TILEMODE_ENTITY, "Draw With &Entities");
    menu_mode->AppendSeparator();
    menu_mode->AppendRadioItem(ID_MENU_PAINTMODE_SQUARES, "Paint &Squares");
    menu_mode->AppendRadioItem(ID_MENU_PAINTMODE_BOXES,   "Paint &Boxess");
    menu_mode->AppendRadioItem(ID_MENU_PAINTMODE_LINES,   "Paint &Lines");
    menu_mode->AppendRadioItem(ID_MENU_PAINTMODE_ROOMS,   "Paint &Rooms");

    wxMenu *menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menu_file, "&File");
    menuBar->Append(menu_mode, "&Mode");
    menuBar->Append(menu_help, "&Help");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuNew,  this, ID_MENU_FILE_NEW);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuExit, this, wxID_EXIT);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuDrawModeTiles, this, ID_MENU_PAINTMODE_SQUARES);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuDrawModeLine,  this, ID_MENU_PAINTMODE_LINES);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuDrawModeBox,   this, ID_MENU_PAINTMODE_BOXES);
    Bind(wxEVT_MENU, &MyMainFrame::onMenuDrawModeRoom,  this, ID_MENU_PAINTMODE_ROOMS);

    Bind(wxEVT_MENU, &MyMainFrame::onMenuAbout, this, wxID_ABOUT);
}


// Create our tool bar.
void MyMainFrame::myCreateToolBar()
{
    wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT | /* wxTB_TEXT | */ wxTB_HORZ_LAYOUT);

    // DEBUG: Apparently you need to add real bitmaps.
    // NEXT UP! Add resources.

    toolbar->AddRadioTool(
        ID_TILEMODE_WALL_BTN, wxT("Walls"), *toolbar_bitmap_wall, wxNullBitmap,
        wxT("Draw Walls"));
    toolbar->AddRadioTool(
        ID_TILEMODE_FLOOR_BTN, wxT("Floors"), *toolbar_bitmap_floor, wxNullBitmap,
        wxT("Draw Floors"));
    toolbar->AddRadioTool(
        ID_TILEMODE_ENTITY_BTN, wxT("Entities"), *toolbar_bitmap_entity, wxNullBitmap,
        wxT("Draw Entities"));
    toolbar->Realize();

    Bind(wxEVT_TOOL, &MyMainFrame::onTileModeWallPress, this, ID_TILEMODE_WALL_BTN);
    Bind(wxEVT_TOOL, &MyMainFrame::onTileModeFloorPress, this, ID_TILEMODE_FLOOR_BTN);
    Bind(wxEVT_TOOL, &MyMainFrame::onTileModeEntityPress, this, ID_TILEMODE_ENTITY_BTN);
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

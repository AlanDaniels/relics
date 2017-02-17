#pragma once

#include "stdafx.h"
#include "resource.h"


enum  BlockType;
class GameLevel;
class MyCanvas;


// IDs for controls.
enum {
    ID_MENU_FILE_NEW,
    ID_MENU_TILEMODE_WALL,
    ID_MENU_TILEMODE_FLOOR,
    ID_MENU_TILEMODE_ENTITY,
    ID_MENU_PAINTMODE_SQUARES,
    ID_MENU_PAINTMODE_LINES,
    ID_MENU_PAINTMODE_BOXES,
    ID_MENU_PAINTMODE_ROOMS,

    ID_TILEMODE_WALL_BTN,
    ID_TILEMODE_FLOOR_BTN,
    ID_TILEMODE_ENTITY_BTN,

    ID_LEVEL_LIST,
    ID_CANVAS,
    ID_ENTITY_LIST
};


// The main app.
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};


// Our different ways of painting.
enum TileMode
{
    TILE_MODE_WALL,
    TILE_MODE_FLOOR,
    TILE_MODE_ENTITY
};

enum DrawMode
{
    DRAW_MODE_BOX,
    DRAW_MODE_LINE,
    DRAW_MODE_TILES,
    DRAW_MODE_ROOM
};


// Everything in the toolbar is just to manipulate these settings.
class MySettings
{
public:
    MySettings();

    TileMode getTileMode() const { return m_tile_mode; }
    DrawMode getDrawMode() const { return m_draw_mode; }

    void setTileMode(TileMode val) { m_tile_mode = val; }
    void setDrawMode(DrawMode val) { m_draw_mode = val; }

private:
    TileMode m_tile_mode;
    DrawMode m_draw_mode;
};


// The main frame window.
class MyMainFrame : public wxFrame
{
public:
    MyMainFrame();
    ~MyMainFrame();

    const MySettings &getSettings() const { return m_settings; }

private:
    // Forbid copying.
    MyMainFrame(const MyMainFrame &that) = delete;
    void operator=(const MyMainFrame &that) = delete;

    void myCreateMenuBar();
    void myCreateToolBar();

    void onMenuNew(wxCommandEvent &evt);
    void onMenuExit(wxCommandEvent &evt);

    inline void onTileModeWallPress(wxCommandEvent &evt)   { m_settings.setTileMode(TILE_MODE_WALL); }
    inline void onTileModeFloorPress(wxCommandEvent &evt)  { m_settings.setTileMode(TILE_MODE_FLOOR); }
    inline void onTileModeEntityPress(wxCommandEvent &evt) { m_settings.setTileMode(TILE_MODE_ENTITY); }

    inline void onMenuDrawModeTiles(wxCommandEvent &evt) { m_settings.setDrawMode(DRAW_MODE_TILES); }
    inline void onMenuDrawModeLine(wxCommandEvent &evt)  { m_settings.setDrawMode(DRAW_MODE_LINE); }
    inline void onMenuDrawModeBox(wxCommandEvent &evt)   { m_settings.setDrawMode(DRAW_MODE_BOX); }
    inline void onMenuDrawModeRoom(wxCommandEvent &evt)  { m_settings.setDrawMode(DRAW_MODE_ROOM); }

    void onMenuAbout(wxCommandEvent &evt);

    wxListBox  *m_level_list;
    MyCanvas   *m_canvas;
    wxListBox  *m_entity_list;

    GameLevel *m_game_world;
    MySettings m_settings;
};

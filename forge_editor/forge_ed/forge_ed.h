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


// What do you want to draw?
enum DrawWhat
{
    DRAW_WHAT_WALL,
    DRAW_WHAT_FLOOR,
    DRAW_WHAT_ENTITY
};


// How do you want to draw it?
enum DrawHow
{
    DRAW_HOW_BOX,
    DRAW_HOW_LINE,
    DRAW_HOW_TILES,
    DRAW_HOW_ROOM
};


// Everything in the toolbar is just to manipulate these settings.
class MySettings
{
public:
    MySettings();

    DrawWhat getDrawWhat() const { return m_draw_what; }
    DrawHow  getDrawHow()  const { return m_draw_how; }

    void setDrawWhat(DrawWhat val) { m_draw_what = val; }
    void setDrawHow(DrawHow val)   { m_draw_how = val; }

private:
    DrawWhat m_draw_what;
    DrawHow m_draw_how;
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
    void populateEntityList();

    void onMenuNew(wxCommandEvent &evt);
    void onMenuExit(wxCommandEvent &evt);

    inline void onMenuDrawModeTiles(wxCommandEvent &evt) { m_settings.setDrawHow(DRAW_HOW_TILES); }
    inline void onMenuDrawModeLine(wxCommandEvent &evt)  { m_settings.setDrawHow(DRAW_HOW_LINE); }
    inline void onMenuDrawModeBox(wxCommandEvent &evt)   { m_settings.setDrawHow(DRAW_HOW_BOX); }
    inline void onMenuDrawModeRoom(wxCommandEvent &evt)  { m_settings.setDrawHow(DRAW_HOW_ROOM); }

    void onMenuAbout(wxCommandEvent &evt);

    wxListBox  *m_level_list;
    MyCanvas   *m_canvas;
    wxListCtrl *m_entity_list;

    GameLevel *m_game_world;
    MySettings m_settings;
};

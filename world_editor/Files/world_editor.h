#pragma once

#include "stdafx.h"


class MyCanvas;
class WorldData;


// IDs for menus and controls.
enum {
    ID_MENU_NEW,
    ID_MENU_OPEN,
    ID_MENU_SAVE,
    ID_MENU_SAVE_AS,
    ID_MENU_CHANGE_IMAGE,
    ID_CANVAS,
};


// The main app.
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int  OnExit();
};


// The main frame window.
class MyMainFrame : public wxFrame
{
public:
    MyMainFrame();
    ~MyMainFrame();

    WorldData *getWorldData() { return m_world_data; }

private:
    // Forbid copying.
    MyMainFrame(const MyMainFrame &that) = delete;
    void operator=(const MyMainFrame &that) = delete;

    void myCreateMenuBar();

    void onMenuNew(wxCommandEvent &evt);
    void onMenuOpen(wxCommandEvent &evt);
    void onMenuSave(wxCommandEvent &evt);
    void onMenuSaveAs(wxCommandEvent &evt);
    void onMenuExit(wxCommandEvent &evt);
    void onMenuChangeImage(wxCommandEvent &evt);
    void onMenuAbout(wxCommandEvent &evt);

    WorldData *m_world_data;
    MyCanvas  *m_canvas;
};

#pragma once

#include "stdafx.h"


class MyCanvas;
class WorldData;


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
    // Disallow copying and moving.
    MyMainFrame(const MyMainFrame &that) = delete;
    void operator=(const MyMainFrame &that) = delete;
    MyMainFrame(MyMainFrame &&that) = delete;
    void operator=(MyMainFrame &&that) = delete;

    // Private methods.
    void myCreateMenuBar();

    void onMenuNew(wxCommandEvent &evt);
    void onMenuOpen(wxCommandEvent &evt);
    void onMenuSave(wxCommandEvent &evt);
    void onMenuSaveAs(wxCommandEvent &evt);
    void onMenuExit(wxCommandEvent &evt);
    void onMenuChangeImage(wxCommandEvent &evt);
    void onMenuAbout(wxCommandEvent &evt);

    // Private data.
    WorldData *m_world_data;
    MyCanvas  *m_canvas;
};

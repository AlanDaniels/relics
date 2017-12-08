#pragma once

#include "stdafx.h"
#include "resource.h"


class MyCanvas;


// IDs for controls.
enum {
    ID_MENU_FILE_NEW,

    ID_LEVEL_LIST,
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

private:
    // Forbid copying.
    MyMainFrame(const MyMainFrame &that) = delete;
    void operator=(const MyMainFrame &that) = delete;

    void myCreateMenuBar();

    void onMenuNew(wxCommandEvent &evt);
    void onMenuExit(wxCommandEvent &evt);
    void onMenuAbout(wxCommandEvent &evt);

    MyCanvas *m_canvas;
};

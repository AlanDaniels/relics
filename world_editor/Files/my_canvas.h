
#pragma once

#include "stdafx.h"


class MyMainFrame;


// The canvas for drawing the game world.
class MyCanvas : public wxPanel
{
public:
    MyCanvas(MyMainFrame *parent);

    int X_worldToScreen(int x) const;
    int Y_worldToScreen(int y) const;

    int X_screenToWorld(int x) const;
    int Y_screenToWorld(int y) const;

    void onSizeEvent(wxSizeEvent& evt);
    void onKeyDownEvent(wxKeyEvent &evt);
    void onKeyUpEvent(wxKeyEvent &evt);
    void onLeftDownEvent(wxMouseEvent &evt);
    void onLeftUpEvent(wxMouseEvent &evt);
    void onMouseMoveEvent(wxMouseEvent &evt);
    void onMouseWheelEvent(wxMouseEvent &evt);

    void onPaintEvent(wxPaintEvent& evt);
    void repaintCanvas();
    void render(wxDC &dc);

    DECLARE_EVENT_TABLE()

private:
    // Disallow copying, and default ctor.
    MyCanvas() = delete;
    MyCanvas(const MyCanvas &that) = delete;
    void operator=(const MyCanvas &that) = delete;

    // Misc code below here.
    void renderGrid(wxDC &dc);

    MyMainFrame *m_parent;

    bool m_panning_mode;
    int  m_zoom_scale;

    int  m_center_x;
    int  m_center_y;
    int  m_old_mouse_x;
    int  m_old_mouse_y;
};

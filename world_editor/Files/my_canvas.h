
#pragma once

#include "stdafx.h"


class MyMainFrame;


// The canvas for drawing the game world.
class MyCanvas : public wxPanel
{
public:
    MyCanvas(MyMainFrame *parent);
    ~MyCanvas();

    int X_worldToScreen(int x) const;
    int Y_worldToScreen(int y) const;

    int X_screenToWorld(int x) const;
    int Y_screenToWorld(int y) const;

    void onSize(wxSizeEvent& evt);
    void onKeyDown(wxKeyEvent &evt);
    void onKeyUp(wxKeyEvent &evt);
    void onMouseLeftDown(wxMouseEvent &evt);
    void onMouseLeftUp(wxMouseEvent &evt);
    void onMouseMove(wxMouseEvent &evt);
    void onMouseWheel(wxMouseEvent &evt);

    void onPaint(wxPaintEvent& evt);
    void repaintCanvas();
    void render(wxDC &dc);

    DECLARE_EVENT_TABLE()

private:
    // Disallow copying, and default ctor.
    MyCanvas() = delete;
    MyCanvas(const MyCanvas &that) = delete;
    void operator=(const MyCanvas &that) = delete;

    void changeZoomScale(bool positive);
    void rebuildZoomedHeightmap();
    void renderWorldData(WorldData *world_data, wxDC &dc);
    void renderGrid(wxDC &dc);

    MyMainFrame *m_parent;

    bool m_panning_mode;
    int  m_zoom_scale;

    int  m_center_x;
    int  m_center_y;
    int  m_old_mouse_x;
    int  m_old_mouse_y;

    int m_old_zoom_scale;
    const wxBitmap *m_old_heightmap;
    wxBitmap *m_zoomed_heightmap;
};

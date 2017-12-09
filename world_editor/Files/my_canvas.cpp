
#include "stdafx.h"

#include "world_editor.h"
#include "my_canvas.h"
#include "world_data.h"


// Declare all our canvas events.
BEGIN_EVENT_TABLE(MyCanvas, wxPanel)
    EVT_SIZE(MyCanvas::onSizeEvent)
    EVT_KEY_DOWN(MyCanvas::onKeyDownEvent)
    EVT_KEY_UP(MyCanvas::onKeyUpEvent)
    EVT_LEFT_DOWN(MyCanvas::onLeftDownEvent)
    EVT_LEFT_UP(MyCanvas::onLeftUpEvent)
    EVT_MOTION(MyCanvas::onMouseMoveEvent)
    EVT_MOUSEWHEEL(MyCanvas::onMouseWheelEvent)
    EVT_PAINT(MyCanvas::onPaintEvent)
END_EVENT_TABLE();


// Static constants for the file here.
static const int INITIAL_ZOOM_SCALE = 24;
static const int MIN_ZOOM_SCALE = 4;

static const wxColor COLOR_GRID_ORIGIN(255, 0, 255);
static const wxColor COLOR_GRID_MAJOR(128, 128, 128);
static const wxColor COLOR_GRID_MINOR(64, 64, 64);

static const wxColor COLOR_TILE_WALL(255, 255, 255);
static const wxColor COLOR_TILE_FLOOR(192, 192, 192);


// Canvas ctor.
MyCanvas::MyCanvas(MyMainFrame *parent) :
    wxPanel(parent),
    m_parent(parent),
    m_panning_mode(false),
    m_zoom_scale(INITIAL_ZOOM_SCALE),
    m_center_x(0),
    m_center_y(0),
    m_old_mouse_x(0),
    m_old_mouse_y(0)
{
}


int MyCanvas::X_worldToScreen(int x) const 
{
    int width = GetSize().GetWidth();
    int center = (width / 2) - m_center_x;
    return center + (x * m_zoom_scale);
}


int MyCanvas::Y_worldToScreen(int y) const 
{
    int height = GetSize().GetHeight();
    int center = (height / 2) - m_center_y;
    return center - (y * m_zoom_scale);
}


int MyCanvas::X_screenToWorld(int x) const
{
    int width = GetSize().GetWidth();
    int center = (width / 2) - m_center_x;
    if ((x - center) >= 0) {
        return (x - center) / m_zoom_scale;
    }
    else {
        return ((x - center) / m_zoom_scale) - 1;
    }
}


int MyCanvas::Y_screenToWorld(int y) const
{
    int height = GetSize().GetHeight();
    int center = (height / 2) - m_center_y;
    if ((y - center) < 0) {
        return -(y - center) / m_zoom_scale;
    }
    else {
        return (-(y - center) / m_zoom_scale) - 1;
    }
}


// When we resize, just repaint everything.B
void MyCanvas::onSizeEvent(wxSizeEvent &evt)
{
    repaintCanvas();
}


// Key down event.
void MyCanvas::onKeyDownEvent(wxKeyEvent &evt)
{
}


// Key up event.
void MyCanvas::onKeyUpEvent(wxKeyEvent &evt)
{
}


// On left mouse down.
void MyCanvas::onLeftDownEvent(wxMouseEvent &evt)
{
}


// On left mouse up.
void MyCanvas::onLeftUpEvent(wxMouseEvent &evt)
{
}


// Mouse move event.
void MyCanvas::onMouseMoveEvent(wxMouseEvent &evt)
{
}


// Let the mouse wheel zoom in and out.
void MyCanvas::onMouseWheelEvent(wxMouseEvent &evt)
{
}


// Handle the paint event.
void MyCanvas::onPaintEvent(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    wxBufferedDC buff_dc(&dc);
    render(buff_dc);
}


// Repaint the entire canvas.
void MyCanvas::repaintCanvas()
{
    wxClientDC dc(this);
    wxBufferedDC buff_dc(&dc);
    render(buff_dc);
}


// Repaint everything.
void MyCanvas::render(wxDC &dc)
{
    // Clear the background.
    wxSize size = GetSize();
    int width  = size.GetWidth();
    int height = size.GetHeight();

    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(0, 0, width, height);

    // Draw everything.
    renderGameData(dc);
    renderGrid(dc);
}


void MyCanvas::renderGameData(wxDC &dc)
{
    WorldData *world_data = m_parent->getWorldData();
    if (world_data == nullptr) {
        return;
    }

    wxBitmap *bitmap = world_data->getBitmap();
    dc.DrawBitmap(*bitmap, wxPoint(0, 0), false);
}

void MyCanvas::renderGrid(wxDC &dc)
{
    wxSize size = GetSize();
    int width  = size.GetWidth();
    int height = size.GetHeight();

    int world_x_min = X_screenToWorld(0);
    int world_x_max = X_screenToWorld(width);

    int world_y_min = Y_screenToWorld(height);
    int world_y_max = Y_screenToWorld(0);

    // Build our pens.
    wxPen *origin_pen = wxThePenList->FindOrCreatePen(COLOR_GRID_ORIGIN, 1, wxPENSTYLE_SOLID);
    wxPen *major_pen  = wxThePenList->FindOrCreatePen(COLOR_GRID_MAJOR,  1, wxPENSTYLE_DOT);
    wxPen *minor_pen  = wxThePenList->FindOrCreatePen(COLOR_GRID_MINOR,  1, wxPENSTYLE_DOT);

    // Draw our vertical grid lines.
    for (int x = world_x_min; x <= world_x_max; x++) {
        if (x == 0) {
            int screen_x = X_worldToScreen(x);
            dc.SetPen(*origin_pen);
            dc.DrawLine(screen_x, 0, screen_x, height);
        }
        else if ((x % 32) == 0) {
            int screen_x = X_worldToScreen(x);
            dc.SetPen(*major_pen);
            dc.DrawLine(screen_x, 0, screen_x, height);
        }
        else if ((x % 4) == 0) {
            int screen_x = X_worldToScreen(x);
            dc.SetPen(*minor_pen);
            dc.DrawLine(screen_x, 0, screen_x, height);
        }
    }

    // Draw our horizontal grid lines.
    for (int y = world_y_min; y <= world_y_max; y++) {
        if (y == 0) {
            int screen_y = Y_worldToScreen(y);
            dc.SetPen(*origin_pen);
            dc.DrawLine(0, screen_y, width, screen_y);
        }
        else if ((y % 32) == 0) {
            int screen_y = Y_worldToScreen(y);
            dc.SetPen(*major_pen);
            dc.DrawLine(0, screen_y, width, screen_y);
        }
        else if ((y % 4) == 0) {
            int screen_y = Y_worldToScreen(y);
            dc.SetPen(*minor_pen);
            dc.DrawLine(0, screen_y, width, screen_y);
        }
    }
}

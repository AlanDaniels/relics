
#include "stdafx.h"

#include "world_editor.h"
#include "my_canvas.h"
#include "world_data.h"


// Declare all our canvas events.
BEGIN_EVENT_TABLE(MyCanvas, wxPanel)
    EVT_SIZE(MyCanvas::onSize)
    EVT_KEY_DOWN(MyCanvas::onKeyDown)
    EVT_KEY_UP(MyCanvas::onKeyUp)
    EVT_LEFT_DOWN(MyCanvas::onMouseLeftDown)
    EVT_LEFT_UP(MyCanvas::onMouseLeftUp)
    EVT_MOTION(MyCanvas::onMouseMove)
    EVT_MOUSEWHEEL(MyCanvas::onMouseWheel)
    EVT_PAINT(MyCanvas::onPaint)
END_EVENT_TABLE();


static const int MIN_ZOOM_SCALE = 1;
static const int MAX_ZOOM_SCALE = 16;


static const wxColor COLOR_RED(255, 0, 0);
static const wxColor COLOR_PURPLE(255, 0, 255, 128);


// Canvas ctor.
MyCanvas::MyCanvas(MyMainFrame *parent) :
    wxPanel(parent),
    m_parent(parent),
    m_panning_mode(false),
    m_zoom_scale(MIN_ZOOM_SCALE),
    m_center_x(0),
    m_center_y(0),
    m_old_mouse_x(0),
    m_old_mouse_y(0),
    m_old_zoom_scale(MIN_ZOOM_SCALE),
    m_old_heightmap(nullptr),
    m_zoomed_heightmap(nullptr)
{
}


int MyCanvas::X_worldToScreen(int x) const 
{
    int width  = GetSize().GetWidth();
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
void MyCanvas::onSize(wxSizeEvent &evt)
{
    repaintCanvas();
}


// Key down event.
// TODO: Make a dialog to show these later.
void MyCanvas::onKeyDown(wxKeyEvent &evt)
{
    wxChar ch = evt.GetUnicodeKey();
    switch (ch) {
    case '-': changeZoomScale(-1); break;
    case '=': changeZoomScale(1); break;
    default: break;
    }
    
    repaintCanvas();
}


// Key up event.
void MyCanvas::onKeyUp(wxKeyEvent &evt)
{
}


// On left mouse down.
void MyCanvas::onMouseLeftDown(wxMouseEvent &evt)
{
}


// On left mouse up.
void MyCanvas::onMouseLeftUp(wxMouseEvent &evt)
{
}


// Mouse move event.
void MyCanvas::onMouseMove(wxMouseEvent &evt)
{
#if 0
    int x = evt.GetX();
    int y = evt.GetY();

    int screen_x = X_screenToWorld(x);
    int screen_y = Y_screenToWorld(y);

    char msg[64];
    sprintf(msg, "Pos: %d, %d", screen_x, screen_y);
    SetToolTip(wxString(msg));
#endif
}


// Let the mouse wheel zoom in and out.
// Note that "wheel rotation" is just a positive or negative.
void MyCanvas::onMouseWheel(wxMouseEvent &evt)
{
    int delta = evt.GetWheelRotation();
    bool positive = (delta > 0);
    changeZoomScale(positive);
    repaintCanvas();
}


// Handle the paint event.
void MyCanvas::onPaint(wxPaintEvent &evt)
{
    repaintCanvas();
}


// Repaint the entire canvas.
void MyCanvas::repaintCanvas()
{
    wxClientDC dc(this);
    wxBufferedDC buff_dc(&dc);
    render(buff_dc);
}


// Change the zoom scale, and clamp it to the allowed values.
void MyCanvas::changeZoomScale(bool positive)
{
    int new_value = m_zoom_scale + (positive ? 1 : -1);
    if (new_value < MIN_ZOOM_SCALE) {
        new_value = MIN_ZOOM_SCALE;
    }
    else if (new_value > MAX_ZOOM_SCALE) {
        new_value = MAX_ZOOM_SCALE;
    }

    if (m_zoom_scale != new_value) {
        m_zoom_scale = new_value;
        rebuildZoomedHeightmap();
    }
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

    // If we have a world loaded, draw it.
    WorldData *world_data = m_parent->getWorldData();
    if (world_data != nullptr) {
        renderWorldData(world_data, dc);
        renderGrid(dc);
    }
}


// Whenever we zoom in on the bitmap, rebuild our cached version.
// Image sizing is expensive, so we don't want to do this if we don't need to.
void MyCanvas::rebuildZoomedHeightmap()
{
    // If there's no loaded world, don't bother.
    WorldData *world_data = m_parent->getWorldData();
    if (world_data == nullptr) {
        return;
    }

    // If we've already built this, and nothing has changed, don't bother.
    wxBitmap *heightmap = world_data->getBitmap();
    if ((m_zoomed_heightmap != nullptr) &&
        (m_old_heightmap  == heightmap) &&
        (m_old_zoom_scale == m_zoom_scale)) {
        return;
    }

    // Free up any previous one.
    if (m_zoomed_heightmap != nullptr) {
        m_zoomed_heightmap->FreeResource();
        m_zoomed_heightmap = nullptr;
    }

    // Build our bigger image.
    int width  = heightmap->GetWidth();
    int height = heightmap->GetHeight();

    wxImage image = heightmap->ConvertToImage();
    m_zoomed_heightmap = new wxBitmap(
        image.Scale(width  * m_zoom_scale,
                    height * m_zoom_scale,
                    wxIMAGE_QUALITY_NEAREST));

    // Remember when to do this again.
    m_old_zoom_scale = m_zoom_scale;
    m_old_heightmap  = heightmap;
}


// And cache the image. Shit gets expensive.
void MyCanvas::renderWorldData(WorldData *world_data, wxDC &dc)
{
    wxBitmap *bitmap = world_data->getBitmap();
    int width  = bitmap->GetWidth();
    int height = bitmap->GetHeight();
    rebuildZoomedHeightmap();

    // Offset the drawing so the center of the bitmap is at the screen origin.
    int screen_x = X_worldToScreen(-width  / 2);
    int screen_y = Y_worldToScreen( height / 2);

    // But, the DC doesn't know about our zoom scale, so draw the zoomed version.
    dc.DrawBitmap(*m_zoomed_heightmap, wxPoint(screen_x, screen_y), false);

    // Draw a red outline around the heightmap.
    wxPen *pen = wxThePenList->FindOrCreatePen(COLOR_RED, 1, wxPENSTYLE_SOLID);
    dc.SetPen(*pen);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(wxRect(screen_x, screen_y,
                            width  * m_zoom_scale,
                            height * m_zoom_scale));
}


void MyCanvas::renderGrid(wxDC &dc)
{
    wxSize size = GetSize();
    int width = size.GetWidth();
    int height = size.GetHeight();

    wxPen *pen = wxThePenList->FindOrCreatePen(COLOR_PURPLE, 1, wxPENSTYLE_SOLID);
    dc.SetPen(*pen);

    // Draw our vertical grid lines.
    int screen_x = X_worldToScreen(0);
    dc.DrawLine(screen_x, 0, screen_x, height);

    // Draw our horizontal grid lines.
    int screen_y = Y_worldToScreen(0);
    dc.DrawLine(0, screen_y, width, screen_y);
}


// This is too useful to get rid of. We'll add it back one day.
#if 0
static const wxColor COLOR_GRID_ORIGIN(255, 0, 255);
static const wxColor COLOR_GRID_MAJOR(128, 128, 128);
static const wxColor COLOR_GRID_MINOR(64, 64, 64);

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
#endif

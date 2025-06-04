
#include "stdafx.h"
#include "my_canvas.h"

#include "game_world.h"
#include "forge_ed.h"


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


// What do we want to draw?
DrawWhat MyCanvas::getDrawWhat() const
{
    return m_parent->getSettings().getDrawWhat();
}


// How do we want to draw it?
DrawHow MyCanvas::getDrawHow() const 
{ 
    return m_parent->getSettings().getDrawHow();
}


BlockType MyCanvas::getBlockType()  const
{
    DrawWhat tile_mode = m_parent->getSettings().getDrawWhat();
    switch (tile_mode) {
    case DRAW_WHAT_WALL:  return BLOCK_TYPE_WALL;
    case DRAW_WHAT_FLOOR: return BLOCK_TYPE_FLOOR;
    default: return BLOCK_TYPE_NONE;
    }
}


// Canvas ctor.
MyCanvas::MyCanvas(MyMainFrame *parent, GameLevel *game_level) :
    wxPanel(parent),
    m_parent(parent),
    m_game_level(game_level),
    m_current_change(nullptr),
    m_panning_mode(false),
    m_zoom_scale(INITIAL_ZOOM_SCALE),
    m_start_pos(0, 0),
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


// Convert the mouse pos into an X-Y world coord.
XYCoord MyCanvas::mousePosToWorld() const
{
    wxPoint pt = wxGetMousePosition();
    int mouse_x = pt.x - this->GetScreenPosition().x;
    int mouse_y = pt.y - this->GetScreenPosition().y;

    int x = X_screenToWorld(mouse_x);
    int y = Y_screenToWorld(mouse_y);
    return XYCoord(x, y);
}


// When we resize, just repaint everything.B
void MyCanvas::onSizeEvent(wxSizeEvent &evt)
{
    repaintCanvas();
}


// Key down event.
void MyCanvas::onKeyDownEvent(wxKeyEvent &evt)
{
    char msg[64];

    int key_code = evt.GetKeyCode();
    auto &settings = m_parent->getSettings();

    // "Space" puts us into panning mode.
    if (key_code == WXK_SPACE) {
        SetCursor(*wxCROSS_CURSOR);
        m_panning_mode = true;
    }

    // "R" resets the view to the origin.
    else if (key_code == 'R') {
        m_center_x = 0;
        m_center_y = 0;
        m_zoom_scale = INITIAL_ZOOM_SCALE;

        sprintf(msg, "Center = [%d, %d]", m_center_x, m_center_y);
        m_parent->SetStatusText(msg);
        repaintCanvas();
    }
}


// Key up event.
void MyCanvas::onKeyUpEvent(wxKeyEvent &evt)
{
    if (evt.GetKeyCode() == WXK_SPACE) {
        SetCursor(*wxSTANDARD_CURSOR);
        m_panning_mode = false;
    }
}


// On left mouse down.
void MyCanvas::onLeftDownEvent(wxMouseEvent &evt)
{
    // If we're panning, never mind.
    if (m_panning_mode) {
        return;
    }

    // Start a new change set.
    assert(m_current_change == nullptr);
    m_current_change = new ChangeSet(*m_game_level);

    if (getDrawHow() == DRAW_HOW_TILES) {
        XYCoord pt = mousePosToWorld();
        BlockType block_type = getBlockType();
        m_current_change->set(pt, block_type);
        m_start_pos = pt;
    }
    else {
        m_start_pos = mousePosToWorld();
    }
    
    repaintCanvas();
}


// On left mouse up.
void MyCanvas::onLeftUpEvent(wxMouseEvent &evt)
{
    // If we're panning, never mind.
    if (m_panning_mode) {
        return;
    }

    // Finish out the change set.
    if (m_current_change != nullptr) {
        m_game_level->applyChanges(*m_current_change);
        delete m_current_change;
        m_current_change = nullptr;
    }
}


// Mouse move event.
void MyCanvas::onMouseMoveEvent(wxMouseEvent &evt)
{
    char msg[64];

    int x = evt.GetX();
    int y = evt.GetY();

    // If the space bar is pressed, we're scrolling the canvas.
    if (m_panning_mode && evt.LeftIsDown()) {
        m_center_x -= x - m_old_mouse_x;
        m_center_y -= y - m_old_mouse_y;

        sprintf(msg, "Center = [%d, %d]", m_center_x, m_center_y);
        m_parent->SetStatusText(msg);
    }

    // If we're in the middle of drawing, go with that.
    if (evt.LeftIsDown() && (m_current_change != nullptr)) {
        XYCoord pt = mousePosToWorld();
        BlockType block_type = getBlockType();

        DrawHow mode = getDrawHow();
        if (mode == DRAW_HOW_TILES) {
            m_current_change->set(pt, block_type);
        }
        else if (mode == DRAW_HOW_BOX) {
            m_current_change->drawBox(m_start_pos, pt, block_type);
        }
        else if (mode == DRAW_HOW_LINE) {
            m_current_change->drawLine(m_start_pos, pt, block_type);
        }
        else if (mode == DRAW_HOW_ROOM) {
            m_current_change->drawRoom(m_start_pos, pt);
        }
    }

    m_old_mouse_x = x;
    m_old_mouse_y = y;
    repaintCanvas();
}


// Let the mouse wheel zoom in and out.
void MyCanvas::onMouseWheelEvent(wxMouseEvent &evt)
{
    int rotation = evt.GetWheelRotation();
    if (rotation > 0) {
        m_zoom_scale += 2;
    }
    else if (rotation < 0) {
        m_zoom_scale -= 2;
        if (m_zoom_scale < MIN_ZOOM_SCALE) {
            m_zoom_scale = MIN_ZOOM_SCALE;
        }
    }

    repaintCanvas();
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
    renderGrid(dc);
    renderAllTiles(dc);
    renderCurrentChangeSet(dc);
    renderCurrentTileOutline(dc);
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


// Draw all the blocks at this level.
void MyCanvas::renderAllTiles(wxDC &dc)
{
    // Render all of the blocks that could be within view.
    int width  = GetSize().GetWidth();
    int height = GetSize().GetHeight();

    int world_left  = X_screenToWorld(0);
    int world_right = X_screenToWorld(width);

    int world_top    = Y_screenToWorld(height);
    int world_bottom = Y_screenToWorld(0);

    // Iterate through, so that the drawing is faster.
    for (auto iter = m_game_level->begin(); iter != m_game_level->end(); iter++) {
        XYCoord pt = iter->first;
        int x = pt.x();
        int y = pt.y();
        if ((x >= world_left) && (x <= world_right) &&
            (y >= world_top)  && (y <= world_bottom)) 
        {
            int left   = X_worldToScreen(x);
            int right  = X_worldToScreen(x + 1);
            int bottom = Y_worldToScreen(y);
            int top    = Y_worldToScreen(y + 1);

            BlockType block_type = m_game_level->get(pt);
            if (block_type != BLOCK_TYPE_NONE) {
                wxColor color = blockTypeToColor(block_type);
                dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID));
                dc.SetPen(*wxThePenList->FindOrCreatePen(color, 1, wxPENSTYLE_SOLID));
                dc.DrawRectangle(left, bottom, right - left, top - bottom);
            }
        }
    }
}


// Draw all the blocks at this level.
void MyCanvas::renderCurrentChangeSet(wxDC &dc)
{
    // If there is no current change set, don't bother.
    if (m_current_change == nullptr) {
        return;
    }

    // Render all of the blocks that could be within view.
    int width = GetSize().GetWidth();
    int height = GetSize().GetHeight();

    int world_left = X_screenToWorld(0);
    int world_right = X_screenToWorld(width);

    int world_top = Y_screenToWorld(height);
    int world_bottom = Y_screenToWorld(0);

    // Iterate through, so that the drawing is faster.
    for (auto iter = m_current_change->begin(); iter != m_current_change->end(); iter++) {
        XYCoord pt = iter->first;
        int x = pt.x();
        int y = pt.y();
        if ((x >= world_left) && (x <= world_right) &&
            (y >= world_top)  && (y <= world_bottom))
        {
            int left   = X_worldToScreen(x);
            int right  = X_worldToScreen(x + 1);
            int bottom = Y_worldToScreen(y);
            int top    = Y_worldToScreen(y + 1);

            // Draw the current level, as solid.
            BlockType block_type = m_current_change->get(pt);
            if (block_type != BLOCK_TYPE_NONE) {
                wxColor color = blockTypeToColor(block_type);
                dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID));
                dc.SetPen(*wxThePenList->FindOrCreatePen(color, 1, wxPENSTYLE_SOLID));
                dc.DrawRectangle(left, bottom, right - left, top - bottom);
            }
        }
    }
}


// Draw an outline around the grid block beneach us..
void MyCanvas::renderCurrentTileOutline(wxDC &dc)
{
    if (m_panning_mode) {
        return;
    }

    XYCoord pt = mousePosToWorld();

    int left   = X_worldToScreen(pt.x());
    int right  = X_worldToScreen(pt.x() + 1);
    int bottom = Y_worldToScreen(pt.y());
    int top    = Y_worldToScreen(pt.y() + 1);

    wxPen *outline_pen = wxThePenList->FindOrCreatePen(wxColour(255, 0, 255), 2, wxPENSTYLE_SOLID);

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*outline_pen);
    dc.DrawRectangle(left, bottom, right - left, top - bottom);
}


// Given a tile type, determine what color to draw it in.
wxColor MyCanvas::blockTypeToColor(BlockType block_type) const
{
    switch (block_type) {
    case BLOCK_TYPE_WALL:  return COLOR_TILE_WALL;
    case BLOCK_TYPE_FLOOR: return COLOR_TILE_FLOOR;

    // No, you shouldn't call these, since you don't draw them.
    case BLOCK_TYPE_NONE: 
    default:
        assert(false);
        return wxColor(255, 0, 255);
    }
}

#pragma once

#include "stdafx.h"


class GameWorld;
class Renderer;


// This is for sending all the event state as a message.
// This makes for an ugly ctor, but it's better than lots of getters.
class EventStateMsg
{
public:
    EventStateMsg(
        bool move_fwd, bool move_bkwd, bool move_left, bool move_right,
        bool move_up,  bool move_down, bool speed_boost,
        int mouse_diff_x, int mouse_diff_y) :
        m_move_fwd(move_fwd),
        m_move_bkwd(move_bkwd),
        m_move_left(move_left),
        m_move_right(move_right),
        m_move_up(move_up),
        m_move_down(move_down),
        m_speed_boost(speed_boost),
        m_mouse_diff_x(mouse_diff_x),
        m_mouse_diff_y(mouse_diff_y) {}

    inline bool getMoveFwd()    const { return m_move_fwd; }
    inline bool getMoveBkwd()   const { return m_move_bkwd; }
    inline bool getMoveLeft()   const { return m_move_left; }
    inline bool getMoveRight()  const { return m_move_right; }
    inline bool getMoveUp()     const { return m_move_up; }
    inline bool getMoveDown()   const { return m_move_down; }
    inline bool getSpeedBoost() const { return m_speed_boost; }
    inline int  getMouseDiffX() const { return m_mouse_diff_x; }
    inline int  getMouseDiffY() const { return m_mouse_diff_y; }

private:
    FORBID_DEFAULT_CTOR(EventStateMsg)
    FORBID_COPYING(EventStateMsg)
    FORBID_MOVING(EventStateMsg)

    // Private data
    bool m_move_fwd;
    bool m_move_bkwd;
    bool m_move_left;
    bool m_move_right;
    bool m_move_up;
    bool m_move_down;
    bool m_speed_boost;
    int  m_mouse_diff_x;
    int  m_mouse_diff_y;
};


// Let's separate out key and mouse events from the game logic.
// This will include stuff like pausing the game, showing menus, etc.
class EventHandler
{
public:
    EventHandler(sf::RenderWindow &window, GameWorld &world, Renderer &renderer) :
        m_window(window),
        m_game_world(world),
        m_renderer(renderer) {}

    bool onEvent(sf::Event event);
    void onGameTick(int elapsed);

private:
    FORBID_DEFAULT_CTOR(EventHandler)
    FORBID_COPYING(EventHandler)
    FORBID_MOVING(EventHandler)

    void centerMouse();

    sf::RenderWindow &m_window;
    GameWorld &m_game_world;
    Renderer  &m_renderer;
};

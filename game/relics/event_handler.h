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

    bool getMoveFwd()    const { return m_move_fwd; }
    bool getMoveBkwd()   const { return m_move_bkwd; }
    bool getMoveLeft()   const { return m_move_left; }
    bool getMoveRight()  const { return m_move_right; }
    bool getMoveUp()     const { return m_move_up; }
    bool getMoveDown()   const { return m_move_down; }
    bool getSpeedBoost() const { return m_speed_boost; }
    int  getMouseDiffX() const { return m_mouse_diff_x; }
    int  getMouseDiffY() const { return m_mouse_diff_y; }

private:
    // Forbid copying, and default ctor.
    EventStateMsg() = delete;
    EventStateMsg(const EventStateMsg &that) = delete;
    void operator=(const EventStateMsg &that) = delete;

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
    // Forbid copying, and default ctor.
    EventHandler() = delete;
    EventHandler(const EventHandler &that) = delete;
    void operator=(const EventHandler &that) = delete;

    void centerMouse();

    sf::RenderWindow &m_window;
    GameWorld &m_game_world;
    Renderer  &m_renderer;
};

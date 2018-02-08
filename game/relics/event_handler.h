#pragma once

#include "stdafx.h"


class GameWorld;
class Renderer;


// The current event as a message.
struct EventStateMsg
{
public:
    EventStateMsg();

    bool move_fwd;
    bool move_bkwd;
    bool move_left;
    bool move_right;
    bool move_up;
    bool move_down;

    bool speed_boost;
    bool jump;

    int  mouse_diff_x;
    int  mouse_diff_y;

private:
    FORBID_COPYING(EventStateMsg)
    FORBID_MOVING(EventStateMsg)
};


// Let's separate out key and mouse events from the game logic.
// This will include stuff like pausing the game, showing menus, etc.
class EventHandler
{
public:
    EventHandler(sf::RenderWindow &window, GameWorld &world, Renderer &renderer) :
        m_window(window),
        m_game_world(world),
        m_renderer(renderer),
        m_jump_key(false) {}

    bool onEvent(sf::Event event);
    void onGameTick(int elapsed);

private:
    FORBID_DEFAULT_CTOR(EventHandler)
    FORBID_COPYING(EventHandler)
    FORBID_MOVING(EventHandler)

    // Private methods.
    void centerMouse();

    // Private data.
    sf::RenderWindow &m_window;
    GameWorld &m_game_world;
    Renderer  &m_renderer;
    bool m_jump_key;
};

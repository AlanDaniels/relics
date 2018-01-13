#pragma once

#include "stdafx.h"


class  Config;
class  GameWorld;
struct RenderStats;


class HeadsUpDisplay
{
public:
    HeadsUpDisplay(sf::RenderWindow &window) :
        m_window(window) {}

    ~HeadsUpDisplay() {}

    bool init();
    void render(const GameWorld &game_world, const RenderStats &stats, GLfloat fps);

private:
    FORBID_DEFAULT_CTOR(HeadsUpDisplay)
    FORBID_COPYING(HeadsUpDisplay)
    FORBID_MOVING(HeadsUpDisplay)

    sf::RenderWindow &m_window;

    sf::RectangleShape m_crosshair_vert;
    sf::RectangleShape m_crosshair_horz;
    sf::Font m_font;
    sf::Text m_text;
};
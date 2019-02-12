#pragma once

#include "stdafx.h"


class  Config;
class  GameWorld;
struct RenderStats;


void SetHudCollisionLine(const std::string &line);
void SetHudDebugLine(const std::string &line);


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

    // Private methods.
    void drawSecondClock(const GameWorld &game_world);
    void drawChunkDetail(const GameWorld &game_world);

    // Private data.
    sf::RenderWindow &m_window;

    sf::RectangleShape m_crosshair_vert;
    sf::RectangleShape m_crosshair_horz;

    sf::CircleShape    m_second_clock;
    sf::RectangleShape m_second_hand;
    
    sf::Font m_font;
    sf::Text m_debugging_text;
    sf::Text m_chunk_detail_text;
};
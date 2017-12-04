#pragma once

#include "stdafx.h"


class  Config;
class  GameWorld;
struct RenderStats;


class OverlayGUI
{
public:
    OverlayGUI::OverlayGUI(sf::RenderWindow &window) :
        m_window(window) {}

    bool init();
    void render(const GameWorld &game_world, const RenderStats &stats, float fps);

private:
    // Forbid copying.
    OverlayGUI(const OverlayGUI &that) = delete;
    void operator=(const OverlayGUI &that) = delete;

    sf::RenderWindow &m_window;

    sf::RectangleShape m_crosshair_vert;
    sf::RectangleShape m_crosshair_horz;
    sf::Font m_font;
    sf::Text m_text;
};
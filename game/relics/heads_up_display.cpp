
#include "stdafx.h"
#include "heads_up_display.h"

#include "config.h"
#include "game_world.h"
#include "renderer.h"

#include "common_util.h"
#include "utils.h"


// Initialize our stuff.
bool HeadsUpDisplay::init()
{
    const GLfloat CH_THIN = 1.0f;
    const GLfloat CH_LONG = 50.0f;

    std::string font_name = GetConfig().render.hud_font;
    if (!ReadFontResource(&m_font, font_name)) {
        return false;
    }

    sf::Vector2u dims = m_window.getSize();
    int screen_width  = dims.x;
    int screen_height = dims.y;

    m_crosshair_vert.setSize(sf::Vector2f(CH_THIN, CH_LONG));
    m_crosshair_vert.setPosition(sf::Vector2f(
        (screen_width  / 2.0f) - (CH_THIN / 2.0f),
        (screen_height / 2.0f) - (CH_LONG / 2.0f)));
    m_crosshair_vert.setFillColor(sf::Color::White);

    m_crosshair_horz.setSize(sf::Vector2f(CH_LONG, CH_THIN));
    m_crosshair_horz.setPosition(sf::Vector2f(
        (screen_width  / 2.0f) - (CH_LONG / 2.0f),
        (screen_height / 2.0f) - (CH_THIN / 2.0f)));
    m_crosshair_horz.setFillColor(sf::Color::White);

    m_text.setFont(m_font);
    m_text.setCharacterSize(14);
    m_text.setStyle(sf::Text::Regular);
    m_text.setFillColor(sf::Color::Yellow);
    m_text.setOutlineColor(sf::Color::Black);
    m_text.setOutlineThickness(1.0f);
    return true;
}


// Render all the debugging stuff we might be interested in.
// Note that we tend to print them from the longest strings to the shortest.
void HeadsUpDisplay::render(const GameWorld &game_world, const RenderStats &stats, GLfloat fps)
{
    // Set things back to where we can draw again.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    const Config &config = GetConfig();

    const GLfloat text_down = 18.0f;

    // Before drawing the text.
    m_window.pushGLStates();

    m_text.setPosition(10.0f, 0.0f);

    // Print the camera details.
    if (config.debug.hud_camera) {
        MyVec4  camera_pos   = game_world.getCameraPos();
        GLfloat camera_yaw   = game_world.getCameraYaw();
        GLfloat camera_pitch = game_world.getCameraPitch();

        std::string msg = fmt::format(
            "Pos = {0:.0f}, {1:.0f}, {2:.0f}, Yaw = {3:03.1f}, Pitch = {4:02.1f}",
            camera_pos.x(), camera_pos.y(), camera_pos.z(),
            camera_yaw, camera_pitch);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print our hit-test debugging info.
    if (config.debug.hud_hit_test) {
        if (game_world.getHitTestSuccess()) {
            std::string descr = game_world.getHitTestDetail().toDescr();
            std::string msg   = fmt::format("Hit Test: {}", descr);
            m_text.setString(msg);
        }
        else {
            m_text.setString("Hit Test: None");
        }

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print our memory usage.
    if (config.debug.hud_memory_usage) {
        int memory = GetMemoryUsage();
        std::string readable = ReadableNumber(memory);
        std::string msg = fmt::format("Memory: {}", readable);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print our render stats.
    if (config.debug.hud_render_stats) {
        std::string readable = ReadableNumber(stats.triangle_count);
        std::string msg = fmt::format(
            "Render: {0} states, {1} tris",
            stats.state_changes, readable);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print our mouse position.
    if (config.debug.hud_mouse_pos) {
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(m_window);
        std::string msg = fmt::format("Mouse: {0:03d}, {1:03d}", mouse_pos.x, mouse_pos.y);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print the game time.
    if (config.debug.hud_game_clock) {
        int game_time = game_world.getTimeMsecs();
        game_time /= 1000;
        int secs = game_time % 60;
        game_time /= 60;
        int minutes = game_time % 60;

        std::string msg = fmt::format("Time: {0:02d}:{1:02d}", minutes, secs);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print our eval region.
    if (config.debug.hud_eval_region) {
        MyVec4  pos = game_world.getCameraPos();
        int eval_blocks = config.logic.eval_blocks;
        EvalRegion region = WorldPosToEvalRegion(pos, eval_blocks);

        std::string msg = fmt::format(
            "Eval distance: {0} meters, W={1}, E={2}, S={3}, N={4}", 
            eval_blocks * CHUNK_WIDTH, 
            region.west(), region.east(), region.south(), region.north());
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print out our chunk stats.
    if (config.debug.hud_chunk_stats) {
        int in_memory  = game_world.getChunksInMemoryCount();
        int considered = stats.chunks_considered;
        int rendered   = stats.chunks_rendered;
        
        std::string msg = fmt::format(
            "Chunks: In memory = {0}, considered = {1}, rendered = {2}",
            in_memory, considered, rendered);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Print our framerate.
    if (config.debug.hud_framerate) {
        std::string msg = fmt::format("FPS: {:02.1f}", fps);
        m_text.setString(msg);

        m_window.draw(m_text);
        m_text.move(0.0f, text_down);
    }

    // Draw the crosshairs, but only if the game is un-paused.
    if (!game_world.isPaused()) {
        m_window.draw(m_crosshair_vert);
        m_window.draw(m_crosshair_horz);
    }

    // After drawing the text.
    m_window.popGLStates();
}

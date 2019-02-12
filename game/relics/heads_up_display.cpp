
#include "stdafx.h"
#include "heads_up_display.h"

#include "common_util.h"
#include "config.h"
#include "game_world.h"
#include "player.h"
#include "renderer.h"
#include "utils.h"


// Show our collision data. Meh, maybe this needs to be an object one day.
static std::string collision_line;
void SetHudCollisionLine(const std::string &line) {
    collision_line = line;
}


// A one-off expedient hack, to see what's going on.
static std::string debug_line;
void SetHudDebugLine(const std::string &line) {
    debug_line = line;
}


// TODO: Hmm. This might be better in a config file.
const int DEBUGGING_TEXT_LINE_HEIGHT  = 18;
const int DEBUGGING_TEXT_LINE_SPACING = 1;

const int CHUNK_DETAIL_TEXT_LINE_HEIGHT  = 14;
const int CHUNK_DETAIL_TEXT_LINE_SPACING = 0;

const GLfloat CROSSHAIR_THIN  = 1.0f;
const GLfloat CHROSSHAIR_LONG = 50.0f;

const GLfloat SECOND_CLOCK_RADIUS = 30.0f;
const GLfloat SECOND_CLOCK_OFFSET = 10.0f;

const GLfloat CHUNK_DETAIL_OFFSET_X = 200.0f;
const GLfloat CHUNK_DETAIL_OFFSET_Y = 50.0f;


// Initialize our stuff.
bool HeadsUpDisplay::init()
{
    const std::string font_name = GetConfig().render.hud_font;
    if (!ReadFontResource(&m_font, font_name)) {
        return false;
    }

    const sf::Vector2u dims = m_window.getSize();
    const int screen_width  = dims.x;
    const int screen_height = dims.y;

    m_crosshair_vert.setSize(sf::Vector2f(CROSSHAIR_THIN, CHROSSHAIR_LONG));
    m_crosshair_vert.setPosition(sf::Vector2f(
        (screen_width  / 2.0f) - (CROSSHAIR_THIN / 2.0f),
        (screen_height / 2.0f) - (CHROSSHAIR_LONG / 2.0f)));
    m_crosshair_vert.setFillColor(sf::Color::White);

    m_crosshair_horz.setSize(sf::Vector2f(CHROSSHAIR_LONG, CROSSHAIR_THIN));
    m_crosshair_horz.setPosition(sf::Vector2f(
        (screen_width  / 2.0f) - (CHROSSHAIR_LONG / 2.0f),
        (screen_height / 2.0f) - (CROSSHAIR_THIN / 2.0f)));
    m_crosshair_horz.setFillColor(sf::Color::White);

    // The per-second clock to show if the framerate is running smoothly.
    m_second_clock.setRadius(SECOND_CLOCK_RADIUS);
    m_second_clock.setPosition(sf::Vector2f(
        screen_width  - (SECOND_CLOCK_RADIUS * 2) - SECOND_CLOCK_OFFSET,
        screen_height - (SECOND_CLOCK_RADIUS * 2) - SECOND_CLOCK_OFFSET));
    m_second_clock.setOutlineColor(sf::Color::Black);
    m_second_clock.setOutlineThickness(1.0f);
    m_second_clock.setFillColor(sf::Color::White);

    m_second_hand.setSize(sf::Vector2f(3.0f, SECOND_CLOCK_RADIUS));
    m_second_hand.setOutlineColor(sf::Color::Black);
    m_second_hand.setOutlineThickness(0.0f);
    m_second_hand.setFillColor(sf::Color::Black);

    // The text for drawing debug messages.
    m_debugging_text.setFont(m_font);
    m_debugging_text.setCharacterSize(DEBUGGING_TEXT_LINE_HEIGHT);
    m_debugging_text.setStyle(sf::Text::Regular);
    m_debugging_text.setFillColor(sf::Color::Yellow);
    m_debugging_text.setOutlineColor(sf::Color::Black);
    m_debugging_text.setOutlineThickness(1.0f);

    // The text for drawing chunk details.
    m_chunk_detail_text.setFont(m_font);
    m_chunk_detail_text.setCharacterSize(12);
    m_chunk_detail_text.setStyle(sf::Text::Regular);
    m_chunk_detail_text.setFillColor(sf::Color::Yellow);
    m_chunk_detail_text.setOutlineThickness(0.0f);

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

    // Before drawing the text.
    const int move_amount = DEBUGGING_TEXT_LINE_HEIGHT + DEBUGGING_TEXT_LINE_SPACING;

    m_window.pushGLStates();

    m_debugging_text.setPosition(10.0f, 0.0f);

    // Print our one-off debug line (if any).
    if (debug_line != "") {
        m_debugging_text.setString(debug_line);
        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print our hit-test debugging info.
    if (config.debug.hud_hit_test) {
        if (game_world.getHitTestSuccess()) {
            std::string descr = game_world.getHitTestResult().toString();
            std::string msg   = fmt::format("Hit Test: {}", descr);
            m_debugging_text.setString(msg);
        }
        else {
            m_debugging_text.setString("Hit Test: None");
        }

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print our memory usage.
    if (config.debug.hud_memory_usage) {
        int memory = GetMemoryUsage();
        std::string readable = ReadableNumber(memory);
        std::string msg = fmt::format("Memory: {}", readable);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print our render stats.
    if (config.debug.hud_render_stats) {
        std::string readable = ReadableNumber(stats.triangle_count);
        std::string msg = fmt::format(
            "Render: {0} states, {1} tris",
            stats.state_changes, readable);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print our mouse position.
    if (config.debug.hud_mouse_pos) {
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(m_window);
        std::string msg = fmt::format("Mouse: {0:03d}, {1:03d}", mouse_pos.x, mouse_pos.y);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print the player's positition.
    if (config.debug.hud_player_pos) {
        const Player &player = game_world.getPlayer();
        MyVec4  camera_pos   = player.getCameraPos();
        GLfloat camera_yaw   = player.getCameraYaw();
        GLfloat camera_pitch = player.getCameraPitch();

        std::string msg = fmt::format(
            "Pos = {0:.0f}, {1:.0f}, {2:.0f}, Yaw = {3:03.1f}, Pitch = {4:02.1f}",
            camera_pos.x(), camera_pos.y(), camera_pos.z(),
            camera_yaw, camera_pitch);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print the player's collision details.
    if (config.debug.hud_collision) {
        std::string msg = fmt::format("Collision: {}", collision_line);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print the game time.
    if (config.debug.hud_game_clock) {
        int game_time = game_world.getTimeMsecs();
        game_time /= 1000;
        int secs = game_time % 60;
        game_time /= 60;
        int minutes = game_time % 60;

        std::string msg = fmt::format("Time: {0:02d}:{1:02d}", minutes, secs);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print our eval region.
    if (config.debug.hud_eval_region) {
        MyVec4 pos = game_world.getPlayer().getCameraPos();
        int eval_block_count = config.logic.eval_block_count;
        EvalRegion region = WorldPosToEvalRegion(pos, eval_block_count);

        std::string msg = fmt::format(
            "Eval distance: {0} meters, W={1}, E={2}, S={3}, N={4}", 
            eval_block_count * CHUNK_WIDTH, 
            region.west(), region.east(), region.south(), region.north());
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print out our chunk details.
    if (config.debug.hud_chunk_summary) {
        int in_memory  = game_world.getChunksInMemoryCount();
        int considered = stats.chunks_considered;
        int rendered   = stats.chunks_rendered;
        
        std::string msg = fmt::format(
            "Chunks: In memory = {0}, considered = {1}, rendered = {2}",
            in_memory, considered, rendered);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // Print our framerate.
    if (config.debug.hud_framerate) {
        std::string msg = fmt::format("FPS: {:02.1f}", fps);
        m_debugging_text.setString(msg);

        m_window.draw(m_debugging_text);
        m_debugging_text.move(0.0f, move_amount);
    }

    // That's the end of text-based output...
    if (config.debug.hud_second_clock) {
        drawSecondClock(game_world);
    }

    if (config.debug.hud_chunk_details) {
        drawChunkDetail(game_world);
    }

    // Draw the crosshairs, but only if the game is un-paused.
    if (!game_world.isPaused()) {
        m_window.draw(m_crosshair_vert);
        m_window.draw(m_crosshair_horz);
    }

    // After drawing the text.
    m_window.popGLStates();
}


// Draw the second clock. Helps us look for sputtering.
// We draw this in the bottom right corner.
void HeadsUpDisplay::drawSecondClock(const GameWorld &game_world) {
    sf::Vector2u dims = m_window.getSize();
    int screen_width  = dims.x;
    int screen_height = dims.y;

    m_second_hand.setPosition(sf::Vector2f(
        screen_width  - SECOND_CLOCK_RADIUS - SECOND_CLOCK_OFFSET,
        screen_height - SECOND_CLOCK_RADIUS - SECOND_CLOCK_OFFSET));

    int leftover_msecs = game_world.getTimeMsecs() % 1000;
    m_second_hand.setRotation((leftover_msecs / 1000.0f) * 360.0f);

    m_window.draw(m_second_clock);
    m_window.draw(m_second_hand);
}


// Draw all the chunk detail as a list down the right side.
void HeadsUpDisplay::drawChunkDetail(const GameWorld &game_world)
{
    const sf::Vector2u dims = m_window.getSize();
    const int screen_width  = dims.x;

    const int move_amount = CHUNK_DETAIL_TEXT_LINE_HEIGHT + CHUNK_DETAIL_TEXT_LINE_SPACING;

    m_chunk_detail_text.setPosition(sf::Vector2f(
        screen_width - CHUNK_DETAIL_OFFSET_X,
        CHUNK_DETAIL_OFFSET_Y));

    std::vector<ChunkOrigin> origins = game_world.getLoadedChunkOrigins();
    for (const auto &iter : origins) {
        std::string msg = fmt::format("{0}, {1}", iter.debugX(), iter.debugZ());
        m_chunk_detail_text.setString(msg);

        m_window.draw(m_chunk_detail_text);
        m_chunk_detail_text.move(sf::Vector2f(0.0f, move_amount));
    }
}

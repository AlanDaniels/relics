
#include "stdafx.h"
#include "event_handler.h"

#include "config.h"
#include "game_world.h"
#include "player.h"
#include "renderer.h"
#include "resource_pool.h"
#include "utils.h"


void EventHandler::centerMouse()
{
    sf::Vector2u dimensions = m_window.getSize();
    int center_x = static_cast<int>(dimensions.x) / 2;
    int center_y = static_cast<int>(dimensions.y) / 2;

    sf::Mouse::setPosition(sf::Vector2i(center_x, center_y), m_window);
}


// Handle an event.
// Return true if the game should keep running.
bool EventHandler::onEvent(sf::Event event)
{
    // If the game is paused, check for a click on the focused window, to un-pause again.
    if (m_game_world.isPaused()) {
        if ((event.type == sf::Event::MouseButtonPressed) &&
            (event.mouseButton.button == sf::Mouse::Left) &&
            m_window.hasFocus()) {
            centerMouse();
            m_game_world.setPaused(false);
            m_window.setMouseCursorVisible(false);
            return true;
        }
    }

    // If the window is resized, update the OpenGL viewport.
    if (event.type == sf::Event::Resized) {
        glViewport(0, 0, event.size.width, event.size.height);
    }

    // Pause the game if we alt-tab out.
    else if (event.type == sf::Event::LostFocus) {
        m_game_world.setPaused(true);
        m_window.setMouseCursorVisible(true);
    }

    else if (event.type == sf::Event::KeyPressed) {
        // For now, use "F4" to exit. Bye!
        if (event.key.code == sf::Keyboard::F4) {
            return false;
        }

        // Use "F5" to reload our config and resources.
        // It's quite okay to catastrophically exit here.
        // TODO: Tell the game world to dump all of its chunks
        // and reload them from scratch.
        if (event.key.code == sf::Keyboard::F5) {
            if (!LoadConfig()) {
                exit(1);
            }

            ClearResourcePool();
            if (!LoadResourcePool()) {
                exit(1);
            }
        }

        // Use space to trigger our magic global breakpoint.
        else if (event.key.code == sf::Keyboard::Space) {
            MAGIC_BREAKPOINT = true;
        }

        // Escape pauses the game, and shows the mouse.
        else if (event.key.code == sf::Keyboard::Escape) {
            m_game_world.setPaused(true);
            m_window.setMouseCursorVisible(true);
        }

        // "R" alone resets the camera.
        // "Ctrl-R" reloads all shaders and config files.
        else if (event.key.code == sf::Keyboard::R) {
            if (event.key.control) {
                LoadConfig();
                m_renderer.init();
            }
            else {
                m_game_world.setPlayerAtStart();
            }
        }

        // "F12" is toggle "noclip" for now.
        else if (event.key.code == sf::Keyboard::F12) {
            GetConfigRW().debug.noclip = !GetConfig().debug.noclip;
        }

#if 0
        // TODO: There's got to be a better way to do this.
        // Everything from here on down is for normal un-paused behavior.
        GLfloat old_yaw = m_game_world.getCameraYaw();
        GLfloat old_pitch = m_game_world.getCameraPitch();

        // Arrows, for exact rotations by degree. Useful for debugging.
        else if (event.key.code == sf::Keyboard::Left) {
            m_game_world.setCameraYaw(floor(old_yaw - 1.0f));
        }
        else if (event.key.code == sf::Keyboard::Right) {
            m_game_world.setCameraYaw(floor(old_yaw + 1.0f));
        }

        else if (event.key.code == sf::Keyboard::Up) {
            m_game_world.setCameraPitch(floor(old_pitch + 1.0f));
        }
        else if (event.key.code == sf::Keyboard::Down) {
            m_game_world.setCameraPitch(floor(old_pitch - 1.0f));
        }
#endif
    }

    // TEMP: Click to delete blocks!
    else if ((event.type == sf::Event::MouseButtonPressed) && 
             (event.mouseButton.button == sf::Mouse::Left)) {
        m_game_world.deleteBlockInFrontOfUs();
    }

    // Keep going.
    return true;
}


// Handle a game tick.
void EventHandler::onGameTick(int elapsed)
{
    // If the game ispaused, nothing to do.
    if (m_game_world.isPaused()) {
        return;
    }

    // See how far the mouse is from the center of the window.
    sf::Vector2u dimensions = m_window.getSize();
    int center_x = static_cast<int>(dimensions.x) / 2;
    int center_y = static_cast<int>(dimensions.y) / 2;

    sf::Vector2i pos = sf::Mouse::getPosition(m_window);
    int diff_x = pos.x - center_x;
    int diff_y = pos.y - center_y;

    // Different keys correspond to movement.
    bool key_W      = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool key_S      = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool key_A      = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool key_D      = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool key_F      = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
    bool key_V      = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
    bool key_LShift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

    EventStateMsg msg(
        key_W,      // Fwd
        key_S,      // Bkwd
        key_A,      // Left
        key_D,      // Right
        key_F,      // Up
        key_V,      // Down
        key_LShift, // Boost
        diff_x,     // Mouse diff X
        diff_y);    // Mouse diff Y

    // Once the keys are processed, tick the game.
    m_game_world.onGameTick(elapsed, msg);
    centerMouse();
}


#include "stdafx.h"
#include "relics.h"
#include "common_util.h"

#include "config.h"
#include "chunk.h"
#include "chunk_hit_test.h"
#include "chunk_loader.h"
#include "event_handler.h"
#include "game_world.h"
#include "draw_state_pct.h"
#include "overlay_gui.h"
#include "renderer.h"
#include "utils.h"


const int INITIAL_SCREEN_WIDTH  = 1920;
const int INITIAL_SCREEN_HEIGHT = 1080;



// Setting up a debugger callback.
void APIENTRY MyOGLErrorCallback(
	GLenum source_val, GLenum type_val, GLuint id, GLenum severity_val, GLsizei msg_len,
	const GLchar* msg_str, const void* user_param)
{
    std::string finder = msg_str;

    // Certain messages are spammed to death. Just ignore these.
    if (severity_val == GL_DEBUG_SEVERITY_NOTIFICATION) {
        if (finder.find("memory as the source for buffer object operations") != std::string::npos) {
            return;
        }
    }
    
    // Otherwise, print the useful message.
    const char *source_str;
	switch (source_val) {
	case GL_DEBUG_SOURCE_API: source_str = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: source_str = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: source_str = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION: source_str = "This App"; break;
	case GL_DEBUG_SOURCE_OTHER: source_str = "Other"; break;
	default: source_str = "No idea!"; break;
	}

	const char *type_str;
	switch (type_val) {
	case GL_DEBUG_TYPE_ERROR: type_str = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: type_str = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY: type_str = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: type_str = "Performance"; break;
	case GL_DEBUG_TYPE_MARKER: type_str = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: type_str = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP: type_str = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER: type_str = "Other"; break;
	default: type_str = "No idea!"; break;
	}

	const char *sev_str;
	switch (severity_val) {
	case GL_DEBUG_SEVERITY_HIGH:         sev_str = "High";   break;
	case GL_DEBUG_SEVERITY_MEDIUM:       sev_str = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          sev_str = "Low";    break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: sev_str = "Notification"; break;
	default: sev_str = "No idea!"; break;
	}

	PrintDebug(
		"OpenGL: source='%s', type='%s', id=%u, sev='%s'\n    %*s\n",
		source_str, type_str, id, sev_str, msg_len, msg_str);

    // If this is high severity, just grind to a halt right here.
    assert(severity_val != GL_DEBUG_SEVERITY_HIGH);
}


// And away we go. Forgive the Microsoft SAL annotations here. We'll make sure this is portable later.
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,    _In_ int nShowCmd)
{
    // Hello and welcome! Read our config file.
    if (!LoadConfig()) {
        return 1;
    }

	// Open the main window.
	sf::ContextSettings settings_in;
	settings_in.depthBits   = 24;
	settings_in.stencilBits = 8;
	settings_in.antialiasingLevel = 4;
	settings_in.majorVersion = 4;
	settings_in.minorVersion = 5;

    sf::Uint32 style;
    if (GetConfig().window.fullscreen) {
        style = sf::Style::Fullscreen;
    }
    else {
        style = sf::Style::Resize | sf::Style::Titlebar | sf::Style::Close;
    }

	sf::RenderWindow window(sf::VideoMode(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT), "Relics", style, settings_in);

    // See what context settings we actually got.
    if (GetConfig().debug.print_window_context) {
        sf::ContextSettings settings_out = window.getSettings();
        PrintDebug("Window Context Settings:\n");
        PrintDebug("    Depth bits: %d\n", settings_out.depthBits);
        PrintDebug("    Stencil bits: %d\n", settings_out.stencilBits);
        PrintDebug("    AA level: %d\n", settings_out.antialiasingLevel);
        PrintDebug("    Version: %d.%d\n", settings_out.majorVersion, settings_out.minorVersion);
    }

    // Set some window stuff.
    bool vertical_sync = GetConfig().window.vertical_sync;
    window.setVerticalSyncEnabled(vertical_sync);

    window.setMouseCursorVisible(false);
    window.setActive(true);

    // Start the mouse at the center.
    sf::Mouse::setPosition(sf::Vector2i(INITIAL_SCREEN_WIDTH / 2, INITIAL_SCREEN_HEIGHT / 2), window);

    // Fire up GLEW. We need to do this *after* we open the main window.
	GLenum glew_result = glewInit();
	if (glew_result != GLEW_OK) {
		PrintDebug("Call to initiate GLEW failed: %s\n", glewGetErrorString(glew_result));
		return 1;
	}

    // Turn on OpenGL debugging.
    if (GetConfig().debug.opengl) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallback(MyOGLErrorCallback, NULL);
    }

    // Set up initial GL stuff, which we'll never change.
    const char* version = (const char*)glGetString(GL_VERSION);
    PrintDebug("Your OpenGL version is \"%s\".\n", version);

    // TODO: Review all these settings, and put them in the Renderer class.
    glFrontFace(GL_CCW);
    glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
    glClearDepth(500.0f);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // The game world.
    GameWorld *game_world = GameWorld::Create();
    if (game_world == nullptr) {
        PrintDebug("Could not create the game world!\n");
        return 1;
    }

    // The renderer.
    Renderer renderer(window, *game_world);
    if (!renderer.init()) {
        PrintDebug("Could not init the renderer.\n");
        return 1;
    }

    // The event handler.
    EventHandler event_handler(window, *game_world, renderer);

    // The GUI.
    OverlayGUI overlay_gui(window);
    if (!overlay_gui.init()) {
        PrintDebug("Could not initialize the GUI.\n");
        return 1;
    }

    // Our event loop.
    sf::Clock clock;
    int game_time = 0;

    int now = clock.getElapsedTime().asMilliseconds();
    int accumulator = 0;

    int fps_elapsed = 0;
    int fps_frames = 0;
    float fps_snapshot = 0.0f;

    while (window.isOpen()) {

        // Poll for events.
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else {
                event_handler.onEvent(event);
            }
        }

        // When enough time has passed, update the game state.
        int new_time = clock.getElapsedTime().asMilliseconds();
        int elapsed  = new_time - now;
        now = new_time;
        accumulator += elapsed;
        while (accumulator >= FRAME_DELTA_MSECS) {
            event_handler.onGameTick(FRAME_DELTA_MSECS);
            accumulator -= FRAME_DELTA_MSECS;
            game_time   += FRAME_DELTA_MSECS;
        }

        // Only update our FPS stats once a second, to avoid flicker.
        fps_elapsed += elapsed;
        fps_frames++;

        if (fps_elapsed >= 1000) {
            fps_snapshot = fps_frames / (fps_elapsed / 1000.0f);
            fps_elapsed = 0;
            fps_frames = 0;
        }

        // Clear the window. Don't call SFML's "window.clear()", since we want to control this ourselves.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Render the world.
        RenderStats stats = renderer.renderWorld();

        // Render the GUI overlay.
        overlay_gui.render(*game_world, stats, fps_snapshot);

        // All done. Flip to the new results.
        window.display();
    }

    // All done.
    window.setMouseCursorVisible(true);
    return 0;
}

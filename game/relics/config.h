#pragma once

#include "stdafx.h"
#include "utils.h"


struct lua_State;


// We'll have lots of config settings, so let's dispense with
// the formalities, and just go for plain structs for now.
struct ConfigWorld
{
    ConfigWorld() :
        file_name("") {}

    ~ConfigWorld() {}

    DEFAULT_COPYING(ConfigWorld)
    DEFAULT_MOVING(ConfigWorld)

    std::string file_name;
};


struct ConfigDebug
{
    ConfigDebug() :
        opengl(true),
        check_for_leaks(false),

        noclip(false),
        draw_transitions(false),

        hud_framerate(false),
        hud_game_clock(false),
        hud_hit_test(false),
        hud_mouse_pos(false),
        hud_player_pos(false),
        hud_second_clock(false),

        hud_memory_usage(false),
        hud_render_stats(false),
        hud_eval_region(false),
        hud_chunk_stats(false),

        print_draw_state(false),
        print_window_context(false) {}

    ~ConfigDebug() {}

    DEFAULT_COPYING(ConfigDebug)
    DEFAULT_MOVING(ConfigDebug)

    bool opengl;
    bool noclip;

    bool check_for_leaks;
    bool draw_transitions;

    bool hud_framerate;
    bool hud_game_clock;
    bool hud_hit_test;
    bool hud_mouse_pos;
    bool hud_player_pos;
    bool hud_second_clock;

    bool hud_memory_usage;
    bool hud_render_stats;
    bool hud_eval_region;
    bool hud_chunk_stats;

    bool print_draw_state;
    bool print_window_context;
};


struct ConfigWindow
{
    ConfigWindow() :
        width(1024),
        height(768),
        fullscreen(false),
        vertical_sync(true),
        mouse_degrees_per_pixel(0.1f) {}

    ~ConfigWindow() {}

    DEFAULT_COPYING(ConfigWindow)
    DEFAULT_MOVING(ConfigWindow)

    int  width;
    int  height;
    bool fullscreen;
    bool vertical_sync;
    GLfloat mouse_degrees_per_pixel;
};


struct ConfigWavefront
{
    ConfigWavefront() {}

    ~ConfigWavefront() {}

    DEFAULT_COPYING(ConfigWavefront)
    DEFAULT_MOVING(ConfigWavefront)

    std::string vert_shader;
    std::string frag_shader;
};


struct ConfigLandscape
{
    ConfigLandscape() {}

    ~ConfigLandscape() {}

    DEFAULT_COPYING(ConfigLandscape)
    DEFAULT_MOVING(ConfigLandscape)

    std::string vert_shader;
    std::string frag_shader;

    std::string grass_texture;
    std::string dirt_texture;
    std::string stone_texture;
    std::string coal_texture;
    std::string bedrock_texture;
};


struct ConfigSkybox
{
    ConfigSkybox() {}

    ~ConfigSkybox() {}

    DEFAULT_COPYING(ConfigSkybox)
    DEFAULT_MOVING(ConfigSkybox)

    std::string vert_shader;
    std::string frag_shader;

    std::string north_texture;
    std::string south_texture;
    std::string east_texture;
    std::string west_texture;
    std::string top_texture;
    std::string bottom_texture;
};


struct ConfigHitTest
{
    ConfigHitTest() {}

    ~ConfigHitTest() {}

    DEFAULT_COPYING(ConfigHitTest)
    DEFAULT_MOVING(ConfigHitTest)

    std::string vert_shader;
    std::string frag_shader;
    std::string texture;
};


struct ConfigRender
{
    ConfigRender() :
        hud_font(""),
        cull_backfaces(true),
        field_of_view(90.0f),
        near_plane_meters(0.1f),
        far_plane_meters(1000.0f),
        fade_distance_meters(80.0f) {}

    ~ConfigRender() {}

    DEFAULT_COPYING(ConfigRender)
    DEFAULT_MOVING(ConfigRender)

    std::string hud_font;

    bool    cull_backfaces;
    GLfloat field_of_view;
    GLfloat near_plane_meters;
    GLfloat far_plane_meters;
    GLfloat fade_distance_meters;

    ConfigWavefront wavefront;
    ConfigLandscape landscape;
    ConfigSkybox    skybox;
    ConfigHitTest   hit_test;

    GLfloat getNearPlaneCm()    const { return near_plane_meters    * 100.0f; }
    GLfloat getFarPlaneCm()     const { return far_plane_meters     * 100.0f; }
    GLfloat getFadeDistanceCm() const { return fade_distance_meters * 100.0f; }
};


struct ConfigLogic
{
    ConfigLogic() :
        eval_blocks(5),
        hit_test_distance_meters(10.0f),
        player_walk_speed(1.0f),
        player_run_speed(1.0f),
        player_gravity(9.8f) {}

    ~ConfigLogic() {}

    DEFAULT_COPYING(ConfigLogic)
    DEFAULT_MOVING(ConfigLogic)

    GLfloat getHitTestDistanceCm() const { return hit_test_distance_meters * 100.0f; }

    // Note that our OpenGL drawing distance can't be more than what's loaded in the world.
    GLfloat getDrawDistanceCm() const { return eval_blocks * CHUNK_WIDTH * 100.0f; }

    int     eval_blocks;
    GLfloat hit_test_distance_meters;
    GLfloat player_walk_speed;
    GLfloat player_run_speed;
    GLfloat player_gravity;
};


class Config
{
public:
    Config() {}

    ~Config() {}

    DEFAULT_COPYING(Config)
    DEFAULT_MOVING(Config)

    bool loadFromFile();

    ConfigWorld  world;
    ConfigDebug  debug;
    ConfigWindow window;
    ConfigRender render;
    ConfigLogic  logic;

private:
    int clampInt(int val, int min_val, int max_val);
    GLfloat clampFloat(GLfloat val, GLfloat min_val, GLfloat max_val);

    bool getBoolField(lua_State *L, const std::string &field_name, bool default_val);
    std::string getStringField(lua_State *L, const std::string &field_name);

    bool validateResource(const std::string &field, const std::string &val) const;
    bool validate() const;
};


// Get at our one expedient global config object.
// It gets loaded at the beginning, and then never changes.
bool LoadConfig();
const Config &GetConfig();

// Well, maybe changes a little...
Config &GetConfigRW();

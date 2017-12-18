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

    std::string file_name;
};


struct ConfigDebug
{
    ConfigDebug() :
        opengl(true),
        show_camera(false),
        show_framerate(false),
        show_game_clock(false),
        show_hit_test(false),
        show_mouse_pos(false),

        show_render_stats(false),
        show_eval_region(false),
        show_chunk_stats(false),

        print_draw_state(false),
        print_window_context(false),
        noclip_flight_speed(1.0f) {}

    bool opengl;
    bool show_camera;
    bool show_framerate;
    bool show_game_clock;
    bool show_hit_test;
    bool show_mouse_pos;

    bool show_render_stats;
    bool show_eval_region;
    bool show_chunk_stats;

    bool print_draw_state;
    bool print_window_context;
    GLfloat noclip_flight_speed;
};


struct ConfigWindow
{
    ConfigWindow() :
        fullscreen(false),
        vertical_sync(true),
        mouse_degrees_per_pixel(0.1f) {}

    bool    fullscreen;
    bool    vertical_sync;
    GLfloat mouse_degrees_per_pixel;
};


struct ConfigLandscape
{
    ConfigLandscape() {}

    std::string vert_shader;
    std::string frag_shader;

    std::string grass_texture;
    std::string dirt_texture;
    std::string stone_texture;
    std::string bedrock_texture;
};


struct ConfigSky
{
    ConfigSky() {}

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

    std::string vert_shader;
    std::string frag_shader;
};


struct ConfigRender
{
    ConfigRender() :
        cull_backfaces(true),
        field_of_view(90.0f),
        near_plane_meters(0.1f),
        far_plane_meters(1000.0f),
        fade_distance_meters(80.0f) {}

    bool cull_backfaces;

    GLfloat field_of_view;
    GLfloat near_plane_meters;
    GLfloat far_plane_meters;
    GLfloat fade_distance_meters;

    ConfigLandscape landscape;
    ConfigSky       sky;
    ConfigHitTest   hit_test;

    GLfloat getNearPlaneCm()    const { return near_plane_meters    * 100.0f; }
    GLfloat getFarPlaneCm()     const { return far_plane_meters     * 100.0f; }
    GLfloat getFadeDistanceCm() const { return fade_distance_meters * 100.0f; }
};


struct ConfigLogic
{
    ConfigLogic() :
        eval_blocks(5),
        hit_test_distance_meters(10.0f) {}

    int     eval_blocks;
    GLfloat hit_test_distance_meters;

    GLfloat getHitTestDistanceCm() const { return hit_test_distance_meters * 100.0f; }

    // Note that our OpenGL drawing distance can't be more than what's loaded in the world.
    GLfloat getDrawDistanceCm() const { return eval_blocks * CHUNK_WIDTH * 100.0f; }
};


class Config
{
public:
    Config() {}

    bool loadFromFile();

    ConfigWorld  world;
    ConfigDebug  debug;
    ConfigWindow window;
    ConfigRender render;
    ConfigLogic  logic;

private:
    // Forbid copying.
    Config(const Config &that) = delete;
    void operator=(const Config &that) = delete;

    int clampInt(int val, int min_val, int max_val);
    GLfloat clampFloat(GLfloat val, GLfloat min_val, GLfloat max_val);

    bool getBoolField(lua_State *L, const char *field_name, bool default_val);
    std::string getStringField(lua_State *L, const char *field_name);

    bool validateResource(const std::string &field, const std::string &val) const;
    bool validate() const;
};


// Get at our one expedient global config object.
// It gets loaded at the beginning, and then never changes.
bool LoadConfig();
const Config &GetConfig();

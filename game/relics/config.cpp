
#include "stdafx.h"
#include "config.h"
#include "common_util.h"

#include "format.h"
#include "utils.h"
#include "lua.hpp"


static std::string config_fname("relics_config.lua");


// Our one global config object.
static Config g_config;

bool LoadConfig() {
    return g_config.loadFromFile();
}

const Config &GetConfig() {
    return g_config;
}

Config &GetConfigRW() {
    return g_config;
}


// Read our default file.
// For now, let's be insistent that the config file exists, and is correct.
bool Config::loadFromFile()
{
    bool path_exists = DoesResourcePathExist();
    if (!path_exists) {
        PrintDebug(fmt::format(
            "The resource path \"{}\" doesn't exist. Running the game is hopeless.",
            RESOURCE_PATH));
        return false;
    }

    std::string data = ReadTextResource(config_fname);
    if (data.empty()) {
        PrintDebug(fmt::format("The config file \"{}\" is empty.", config_fname));
        return false;
    }

    lua_State *L = luaL_newstate();
    if (L == nullptr) {
        assert(false);
        return false;
    }

    luaL_openlibs(L);

    int val;
    
    val = luaL_loadbuffer(L, data.c_str(), data.length(), "config");
    if (val != LUA_OK) {
        PrintDebug(fmt::format(
            "Lua error when loading {0}: {1}, {2}\n", 
            config_fname, LuaErrorToString(val), lua_tostring(L, -1)));
        lua_close(L);
        return false;
    }

    val = lua_pcall(L, 0, 0, 0);
    if (val != LUA_OK) {
        PrintDebug(fmt::format(
            "Lua error when running {0}: {1}, {2}\n",
            config_fname, LuaErrorToString(val), lua_tostring(L, -1)));
        lua_close(L);
        return false;
    }

    // Read the "world" table.
    lua_getglobal(L, "world");
    if (lua_istable(L, -1)) {
        world.file_name = getStringField(L, "file_name");
    }
    lua_pop(L, 1);

    // Read the "debug" table.
    lua_getglobal(L, "debug");
    if (lua_istable(L, -1)) {
        debug.opengl           = getBoolField(L, "opengl", false);
        debug.noclip           = getBoolField(L, "noclip", false);
        debug.check_for_leaks  = getBoolField(L, "check_for_leaks",  false);
        debug.draw_transitions = getBoolField(L, "draw_transitions", false);

        debug.hud_framerate  = getBoolField(L, "hud_framerate",  false);
        debug.hud_game_clock = getBoolField(L, "hud_game_clock", false);
        debug.hud_hit_test   = getBoolField(L, "hud_hit_test",   false);
        debug.hud_mouse_pos  = getBoolField(L, "hud_mouse_pos",  false);
        debug.hud_player_pos = getBoolField(L, "hud_player_pos", false);
        debug.hud_blinker    = getBoolField(L, "hud_blinker",    false);

        debug.hud_memory_usage = getBoolField(L, "hud_memory_usage", false);
        debug.hud_render_stats = getBoolField(L, "hud_render_stats", false);
        debug.hud_eval_region  = getBoolField(L, "hud_eval_region",  false);
        debug.hud_chunk_stats  = getBoolField(L, "hud_chunk_stats",  false);

        debug.print_draw_state     = getBoolField(L, "print_draw_state",     false);
        debug.print_window_context = getBoolField(L, "print_window_context", false);
    }
    lua_pop(L, 1);

    // Read the "window" table.
    lua_getglobal(L, "window");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "width");
        if (lua_isinteger(L, -1)) {
            int val = static_cast<int>(lua_tointeger(L, -1));
            window.width = clampInt(val, 400, 5000);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "height");
        if (lua_isinteger(L, -1)) {
            int val = static_cast<int>(lua_tointeger(L, -1));
            window.height = clampInt(val, 400, 5000);
        }
        lua_pop(L, 1);

        window.fullscreen    = getBoolField(L, "fullscreen", false);
        window.vertical_sync = getBoolField(L, "vertical_sync", true);

        // Clamp the "mouse degrees per pixel" to reasonable values.
        lua_getfield(L, -1, "mouse_degrees_per_pixel");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            window.mouse_degrees_per_pixel = clampFloat(val, 0.01f, 0.5f);
        }
    }
    lua_pop(L, 1);

    // Read the "render" table.
    lua_getglobal(L, "render");
    if (lua_istable(L, -1)) {
        render.hud_font = getStringField(L, "hud_font");

        render.cull_backfaces = getBoolField(L, "cull_backfaces", true);

        // Clamp the field of view from 30 degrees to 180 degrees.
        lua_getfield(L, -1, "field_of_view");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            render.field_of_view = clampFloat(val, 30.0f, 180.0f);
        }
        lua_pop(L, 1);

        // Clamp the near plane from 5 cm to 1 meter.
        lua_getfield(L, -1, "near_plane");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            render.near_plane_meters = clampFloat(val, 0.05f, 1.0f);
        }
        lua_pop(L, 1);

        // Clamp the far plane from 10 meters to 1000 meters.
        lua_getfield(L, -1, "far_plane");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            render.far_plane_meters = clampFloat(val, 10.0f, 1000.0f);
        }
        lua_pop(L, 1);

        // Clamp the fade distance to be greater than zero.
        lua_getfield(L, -1, "fade_distance");
        if (lua_isnumber(L, -1)) {
            render.fade_distance_meters = static_cast<GLfloat>(lua_tonumber(L, -1));
            if (render.fade_distance_meters < 0.0f) {
                render.fade_distance_meters = 0.0f;
            }
        }
        lua_pop(L, 1);

        // Read the "wavefront" settings.
        lua_getfield(L, -1, "wavefront");
        if (lua_istable(L, -1)) {
            render.wavefront.vert_shader = getStringField(L, "vert_shader");
            render.wavefront.frag_shader = getStringField(L, "frag_shader");
        }
        lua_pop(L, 1);

        // Read the "landscape" settings.
        lua_getfield(L, -1, "landscape");
        if (lua_istable(L, -1)) {
            render.landscape.vert_shader = getStringField(L, "vert_shader");
            render.landscape.frag_shader = getStringField(L, "frag_shader");

            render.landscape.grass_texture   = getStringField(L, "grass_texture");
            render.landscape.dirt_texture    = getStringField(L, "dirt_texture");
            render.landscape.stone_texture   = getStringField(L, "stone_texture");
            render.landscape.coal_texture    = getStringField(L, "coal_texture");
            render.landscape.bedrock_texture = getStringField(L, "bedrock_texture");
        }
        lua_pop(L, 1);

        // Read the "sky" settings.
        lua_getfield(L, -1, "skybox");
        if (lua_istable(L, -1)) {
            render.skybox.vert_shader = getStringField(L, "vert_shader");
            render.skybox.frag_shader = getStringField(L, "frag_shader");

            render.skybox.north_texture  = getStringField(L, "north_texture");
            render.skybox.south_texture  = getStringField(L, "south_texture");
            render.skybox.east_texture   = getStringField(L, "east_texture");
            render.skybox.west_texture   = getStringField(L, "west_texture");
            render.skybox.top_texture    = getStringField(L, "top_texture");
            render.skybox.bottom_texture = getStringField(L, "bottom_texture");
        }
        lua_pop(L, 1);

        // Read the "hit test" settings.
        lua_getfield(L, -1, "hit_test");
        if (lua_istable(L, -1)) {
            render.hit_test.vert_shader = getStringField(L, "vert_shader");
            render.hit_test.frag_shader = getStringField(L, "frag_shader");
            render.hit_test.texture     = getStringField(L, "texture");
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    // Read the "logic" table.
    lua_getglobal(L, "logic");
    if (lua_istable(L, -1)) {

        // Clamp the eval blocks count. We'll clamp from 1 to 6 for now.
        //   A value of 1 => (1 + 2x) ** 2 =>  3 ** 2 =>   9 blocks.
        //   A value of 7 => (1 + 2x) ** 2 => 15 ** 2 => 225 blocks.
        // If we go any bigger than that, we're asking to run out of memory.
        lua_getfield(L, -1, "eval_blocks");
        if (lua_isnumber(L, -1)) {
            int val = static_cast<int>(lua_tointeger(L, -1));
            logic.eval_blocks = clampInt(val, 1, 7);
        }
        lua_pop(L, 1);

        // Clamp the hit-test distance to one meter, to 500 meters.
        lua_getfield(L, -1, "hit_test_distance");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            logic.hit_test_distance_meters = clampFloat(val, 1.0f, 500.f);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "player_walk_speed");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            logic.player_walk_speed = clampFloat(val, 0.001f, 10.f);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "player_run_speed");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            logic.player_run_speed = clampFloat(val, 0.001f, 10.f);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "player_gravity");
        if (lua_isnumber(L, -1)) {
            GLfloat val = static_cast<GLfloat>(lua_tonumber(L, -1));
            logic.player_gravity = clampFloat(val, 0.001f, 50.f);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_close(L);

    // Now that we're done, validate our contents.
    return validate();
}


// Wrap some of the Lua stuff.
bool Config::getBoolField(lua_State *L, const std::string &field_name, bool default_val)
{
    bool result = default_val;
    lua_getfield(L, -1, field_name.c_str());
    if (lua_isboolean(L, -1)) {
        result = (lua_toboolean(L, -1) != 0);
    }
    lua_pop(L, 1);
    return result;
}


// Wrap some of the Lua stuff.
std::string Config::getStringField(lua_State *L, const std::string &field_name)
{
    std::string result = "";
    lua_getfield(L, -1, field_name.c_str());
    if (lua_isstring(L, -1)) {
        result = lua_tostring(L, -1);
    }
    lua_pop(L, 1);
    return result;
}


// Make sure an int stays within reasonable values.
int Config::clampInt(int val, int min_val, int max_val)
{
    if (val < min_val) {
        return min_val;
    }
    else if (val > max_val) {
        return max_val;
    }
    else {
        return val;
    }
}


// Make sure a float stays within reasonable values.
GLfloat Config::clampFloat(GLfloat val, GLfloat min_val, GLfloat max_val)
{
    if (val < min_val) {
        return min_val;
    }
    else if (val > max_val) {
        return max_val;
    }
    else {
        return val;
    }
}


// Validate that a particular resource was loaded.
bool Config::validateResource(const std::string &field_name, const std::string &val) const 
{
    if (val.empty()) {
        PrintDebug(fmt::format("Config field '{}' is not set.\n", field_name));
        return false;
    }

    if (!IsResource(val)) {
        PrintDebug(fmt::format(
            "Config field '{0}' is set to '{2}', which is not a valid resource.\n", 
            field_name, val));
        return false;
    }

    return true;
}


// After we're loaded, validate everything.
bool Config::validate() const 
{
    bool success = true;

    // Big picture stuff. 
    if (!validateResource("world.file_name", world.file_name)) { success = false; }

    // Fonts, textures and shaders.
    if (!validateResource("render.hud_font", render.hud_font)) { success = false; }

    // Wavefront resources.
    if (!validateResource("render.wavefront.vert_shader", render.wavefront.vert_shader)) { success = false; }
    if (!validateResource("render.wavefront.frag_shader", render.wavefront.frag_shader)) { success = false; }

    // Landscape resoures.
    if (!validateResource("render.landscape.vert_shader", render.landscape.vert_shader)) { success = false; }
    if (!validateResource("render.landscape.frag_shader", render.landscape.frag_shader)) { success = false; }

    if (!validateResource("render.landscape.grass_texture",   render.landscape.grass_texture)) { success = false; }
    if (!validateResource("render.landscape.dirt_texture",    render.landscape.dirt_texture))  { success = false; }
    if (!validateResource("render.landscape.stone_texture",   render.landscape.stone_texture)) { success = false; }
    if (!validateResource("render.landscape.coal_texture",    render.landscape.coal_texture)) { success = false; }
    if (!validateResource("render.landscape.bedrock_texture", render.landscape.bedrock_texture)) { success = false; }

    // Skybox resources.
    if (!validateResource("render.sky.vert_shader", render.skybox.vert_shader)) { success = false; }
    if (!validateResource("render.sky.frag_shader", render.skybox.frag_shader)) { success = false; }

    if (!validateResource("render.sky.north_texture",  render.skybox.north_texture))  { success = false; }
    if (!validateResource("render.sky.south_texture",  render.skybox.south_texture))  { success = false; }
    if (!validateResource("render.sky.east_texture",   render.skybox.east_texture))   { success = false; }
    if (!validateResource("render.sky.west_texture",   render.skybox.west_texture))   { success = false; }
    if (!validateResource("render.sky.top_texture",    render.skybox.top_texture))    { success = false; }
    if (!validateResource("render.sky.bottom_texture", render.skybox.bottom_texture)) { success = false; }

    // Hit Test resources.
    if (!validateResource("render.hit_test.vert_shader", render.hit_test.vert_shader)) { success = false; }
    if (!validateResource("render.hit_test.frag_shader", render.hit_test.frag_shader)) { success = false; }
    if (!validateResource("render.hit_test.texture",     render.hit_test.texture))     { success = false; }

    if (!success) {
        PrintDebug(fmt::format("File '{}' has errors. Fix these and try again.\n", config_fname));
    }

    return success;
}

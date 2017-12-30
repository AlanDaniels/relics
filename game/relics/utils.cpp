
#include "stdafx.h"
#include "common_util.h"
#include "relics.h"
#include "utils.h"
#include "lua.hpp"


// A breakpoint to be triggered just once, when we hit the spacebar.
bool MAGIC_BREAKPOINT;


// TODO: One day, turn this into a zip file.
const char *RESOURCE_PATH = "D:\\relics\\resources\\";


// Convert a face enum to a string.
std::string FaceTypeToString(FaceType face_type)
{
    switch (face_type) {
    case FaceType::NONE:   return "none";
    case FaceType::SOUTH:  return "south";
    case FaceType::NORTH:  return "north";
    case FaceType::WEST:   return "west";
    case FaceType::EAST:   return "east";
    case FaceType::TOP:    return "top";
    case FaceType::BOTTOM: return "bottom";
    default: return "unknown";
    }
}


// Convert an edge enum to a string.
std::string EdgeTypeToString(EdgeType edge_type)
{
    switch (edge_type) {
    case EdgeType::NONE:  return "none";
    case EdgeType::UPPER: return "upper";
    case EdgeType::LOWER: return "lower";
    case EdgeType::LEFT:  return "left";
    case EdgeType::RIGHT: return "right";
    default: return "unknown";
    }
}


// Convert a corner enum to a string.
std::string CornerTypeToString(CornerType corner_type)
{
    switch (corner_type) {
    case CornerType::NONE:        return "none";
    case CornerType::LOWER_LEFT:  return "lower-left";
    case CornerType::LOWER_RIGHT: return "lower-right";
    case CornerType::UPPER_LEFT:  return "upper-left";
    case CornerType::UPPER_RIGHT: return "upper-right";
    default: return "unknown";
    }
}


// Convert a grid coord to world coordinates.
GLfloat GridToWorld(int grid)
{
    return grid * BLOCK_SCALE;
}


// Get a south-facing grid plane.
MyPlane GetSouthGridPlane(int grid_z)
{
    return MyPlane(VEC4_SOUTHWARD, -GridToWorld(grid_z));
}


// Get a north-facing grid plane.
MyPlane GetNorthGridPlane(int grid_z)
{
    return MyPlane(VEC4_NORTHWARD, GridToWorld(grid_z));
}


// Get a west-facing grid plane.
MyPlane GetWestGridPlane(int grid_x)
{
    return MyPlane(VEC4_WESTWARD, -GridToWorld(grid_x));
}


// Get an east-facing grid plane.
MyPlane GetEastGridPlane(int grid_x)
{
    return MyPlane(VEC4_EASTWARD, GridToWorld(grid_x));
}


// Get a top-facing grid plane.
MyPlane GetTopGridPlane(int grid_y)
{
    return MyPlane(VEC4_UPWARD, GridToWorld(grid_y));
}


// Get a bottom facing grid plane.
MyPlane GetBottomGridPlane(int grid_y)
{
    return MyPlane(VEC4_DOWNWARD, -GridToWorld(grid_y));
}


// Print all the OpenGL state stuff, so we can see what's going on.
static const char *CapToStr(GLenum cap)
{
    return (glIsEnabled(cap) ? "enabled" : "disabled");
}


void PrintOpenglEnabledState(const char *title, bool details)
{
    PrintDebug("%s - OpenGL flags:\n", title);
    PrintDebug("    BLEND = %s\n", CapToStr(GL_BLEND));
    PrintDebug("    CLIP_DISTANCE0 = %s\n", CapToStr(GL_CLIP_DISTANCE0));
    PrintDebug("    CLIP_DISTANCE1 = %s\n", CapToStr(GL_CLIP_DISTANCE1));
    PrintDebug("    CLIP_DISTANCE2 = %s\n", CapToStr(GL_CLIP_DISTANCE2));
    PrintDebug("    CLIP_DISTANCE3 = %s\n", CapToStr(GL_CLIP_DISTANCE3));
    PrintDebug("    CLIP_DISTANCE4 = %s\n", CapToStr(GL_CLIP_DISTANCE4));
    PrintDebug("    CLIP_DISTANCE5 = %s\n", CapToStr(GL_CLIP_DISTANCE5));
    PrintDebug("    COLOR_LOGIC_OP = %s\n", CapToStr(GL_COLOR_LOGIC_OP));
    PrintDebug("    CULL_FACE      = %s\n", CapToStr(GL_CULL_FACE));
    PrintDebug("    DEBUG_OUTPUT   = %s\n", CapToStr(GL_DEBUG_OUTPUT));
    PrintDebug("    DEBUG_OUTPUT_SYNCHRONOUS = %s\n", CapToStr(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    PrintDebug("    DEPTH_CLAMP = %s\n", CapToStr(GL_DEPTH_CLAMP));
    PrintDebug("    DEPTH_TEST = %s\n", CapToStr(GL_DEPTH_TEST));
    PrintDebug("    DITHER = %s\n", CapToStr(GL_DITHER));
    PrintDebug("    FRAMEBUFFER_SRGB = %s\n", CapToStr(GL_FRAMEBUFFER_SRGB));
    PrintDebug("    LINE_SMOOTH = %s\n", CapToStr(GL_LINE_SMOOTH));
    PrintDebug("    MULTISAMPLE = %s\n", CapToStr(GL_MULTISAMPLE));
    PrintDebug("    POLYGON_OFFSET_FILL = %s\n", CapToStr(GL_POLYGON_OFFSET_FILL));
    PrintDebug("    POLYGON_OFFSET_LINE = %s\n", CapToStr(GL_POLYGON_OFFSET_LINE));
    PrintDebug("    POLYGON_OFFSET_POINT = %s\n", CapToStr(GL_POLYGON_OFFSET_POINT));
    PrintDebug("    POLYGON_SMOOTH = %s\n", CapToStr(GL_POLYGON_SMOOTH));
    PrintDebug("    PRIMITIVE_RESTART = %s\n", CapToStr(GL_PRIMITIVE_RESTART));
    PrintDebug("    PRIMITIVE_RESTART_FIXED_INDEX = %s\n", CapToStr(GL_PRIMITIVE_RESTART_FIXED_INDEX));
    PrintDebug("    RASTERIZER_DISCARD = %s\n", CapToStr(GL_RASTERIZER_DISCARD));
    PrintDebug("    SAMPLE_ALPHA_TO_COVERAGE = %s\n", CapToStr(GL_SAMPLE_ALPHA_TO_COVERAGE));
    PrintDebug("    SAMPLE_ALPHA_TO_ONE = %s\n", CapToStr(GL_SAMPLE_ALPHA_TO_ONE));
    PrintDebug("    SAMPLE_COVERAGE = %s\n", CapToStr(GL_SAMPLE_COVERAGE));
    PrintDebug("    SAMPLE_SHADING = %s\n", CapToStr(GL_SAMPLE_SHADING));
    PrintDebug("    SAMPLE_MASK = %s\n", CapToStr(GL_SAMPLE_MASK));
    PrintDebug("    SCISSOR_TEST = %s\n", CapToStr(GL_SCISSOR_TEST));
    PrintDebug("    STENCIL_TEST = %s\n", CapToStr(GL_STENCIL_TEST));
    PrintDebug("    TEXTURE_CUBE_MAP_SEAMLESS = %s\n", CapToStr(GL_TEXTURE_CUBE_MAP_SEAMLESS));
    PrintDebug("    PROGRAM_POINT_SIZE = %s\n", CapToStr(GL_PROGRAM_POINT_SIZE));

    // Most flags are boring. Only print all of theme if we really want.
    if (details) {
    }
}


// Convert a Lua error value to a string.
const char *LuaErrorToString(int val)
{
    switch (val) {
    case LUA_OK:        return "okay";
    case LUA_ERRSYNTAX: return "syntax error";
    case LUA_ERRMEM:    return "out of memory error";
    case LUA_ERRRUN:    return "runtime error";
    case LUA_ERRGCMM:   return "garbage collector error";
    case LUA_ERRERR:    return "message handler error";
    default:
        PrintTheImpossible(__FILE__, __LINE__, val);
        return "undefined";
    }
}


// Check if our resource path actually exists.
// Only do this once, when we're first loading the config file.
bool DoesResourcePathExist() {
    struct stat info;
    int got_stats = stat(RESOURCE_PATH, &info);
    if (got_stats != 0) {
        return false;
    }

    int dir_flag = (info.st_mode & S_IFDIR);
    return (dir_flag == S_IFDIR);
}


// See if a file exists.
bool IsResource(const std::string &fname) {
    std::string full_name = RESOURCE_PATH;
    full_name.append(fname);

    std::ifstream fs(full_name);
    if (fs.is_open()) {
        fs.close();
        return true;
    }
    else {
        return false;
    } 
}


// Read a text file resource into a string.
std::string ReadTextResource(const std::string &fname)
{
    std::string full_name = RESOURCE_PATH;
    full_name.append(fname);

    std::ifstream fs(full_name);
    if (!fs.is_open()) {
        PrintDebug("Count not open text resource %s.\n", fname.c_str());
        return "";
    }

    std::stringstream buffer;
    buffer << fs.rdbuf();
    fs.close();
    return buffer.str();
}


// Load a font resource.
bool ReadFontResource(sf::Font *pOut, const std::string &fname)
{
    std::string full_name = RESOURCE_PATH;
    full_name.append(fname);

    if (!pOut->loadFromFile(full_name.c_str())) {
        PrintDebug("Could not open font resource %s.'\n", fname.c_str());
        return false;
    }

    return true;
}


// Load an image resource.
bool ReadImageResource(sf::Image *pOut, const std::string &fname)
{
    std::string full_name = RESOURCE_PATH;
    full_name.append(fname);

    // Load the image.
    if (!pOut->loadFromFile(full_name)) {
        PrintDebug("Could not load texture resource %s.\n", fname.c_str());
        return false;
    }

    return true;
}


// Round an integer up to the nearest multiple.
// We've had to roll our own here since negative numbers are tricky.
int RoundUpInt(int val, int mult)
{
    assert(mult > 0);

    int remainder = abs(val) % mult;
    if (remainder == 0) {
        return val;
    }

    if (val >= 0) {
        return val + mult - remainder;
    }
    else {
        return -(abs(val) - remainder);
    }
}


// Round an integer down to the nearest multiple.
// We've had to roll our own here since negative numbers are tricky.
int RoundDownInt(int val, int mult)
{
    assert(mult > 0);

    int remainder = abs(val) % mult;
    if (remainder == 0) {
        return val;
    }

    if (val >= 0) {
        return val - remainder;
    }
    else {
        return -(abs(val) + mult - remainder);
    }
}

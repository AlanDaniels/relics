
#include "stdafx.h"
#include "relics.h"
#include "utils.h"
#include "lua.hpp"


// TODO: One day, turn this into a zip file or something.
static const std::string RESOURCE_PATH = "D:\\Projects\\relics\\my_resources\\";


// Convert a face enum to a string.
std::string FaceEnumToString(FaceEnum face)
{
    switch (face) {
    case FACE_NONE:   return std::string("none");
    case FACE_SOUTH:  return std::string("south");
    case FACE_NORTH:  return std::string("north");
    case FACE_WEST:   return std::string("west");
    case FACE_EAST:   return std::string("east");
    case FACE_TOP:    return std::string("top");
    case FACE_BOTTOM: return std::string("bottom");
    default: return std::string("unknown");
    }
}


// Convert an edge enum to a string.
std::string EdgeEnumToString(EdgeEnum edge)
{
    switch (edge) {
    case EDGE_NONE:  return std::string("none");
    case EDGE_UPPER: return std::string("upper");
    case EDGE_LOWER: return std::string("lower");
    case EDGE_LEFT:  return std::string("left");
    case EDGE_RIGHT: return std::string("right");
    default: return std::string("unknown");
    }
}


// Convert a corner enum to a string.
std::string CornerEnumToString(CornerEnum corner)
{
    switch (corner) {
    case CORNER_NONE:        return std::string("none");
    case CORNER_LOWER_LEFT:  return std::string("lower-left");
    case CORNER_LOWER_RIGHT: return std::string("lower-right");
    case CORNER_UPPER_LEFT:  return std::string("upper-left");
    case CORNER_UPPER_RIGHT: return std::string("upper-right");
    default: return std::string("unknown");
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


// Print a debug message, both to the Visual Studio debugger, and stdout.
// Note the global here means this call won't be thread safe, if you ever get to that point.
static char debug_dest[2048];
void PrintDebug(const char *format, ...)
{
    va_list arg_list;

    va_start(arg_list, format);
    vsprintf(debug_dest, format, arg_list);
    va_end(arg_list);

    OutputDebugStringA(debug_dest);
    printf(debug_dest);
}


// Print when we hit an "impossible" case in a switch statement, and kill the app.
void PrintTheImpossible(const char *fname, int line_num, int value)
{
    PrintDebug(
        "IMPOSSIBLE VALUE! %s (line %d), value of %d\n.",
        fname, line_num, value);
    assert(false);
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


// See if a file exists.
bool IsResource(const std::string &fname) {
    std::string full_name = RESOURCE_PATH;
    full_name.append(fname);

    std::ifstream fs(full_name);
    if (fs.is_open()) {
        fs.close();
        return true;
    } else {
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

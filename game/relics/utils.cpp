
#include "stdafx.h"
#include "common_util.h"

#include "format.h"
#include "relics.h"
#include "utils.h"
#include "lua.hpp"

#include <boost/filesystem.hpp>


// A breakpoint to be triggered just once, when we hit the spacebar.
bool MAGIC_BREAKPOINT;


// TODO: One day, turn this into a zip file.
const std::string RESOURCE_PATH = "D:\\relics\\resources\\";


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
static std::string CapToStr(GLenum cap)
{
    return (glIsEnabled(cap) ? "enabled" : "disabled");
}


void PrintOpenglEnabledState(const std::string &title, bool details)
{
    PrintDebug(fmt::format("{} - OpenGL flags:\n", title));
    PrintDebug(fmt::format("    BLEND = {}\n", CapToStr(GL_BLEND)));
    PrintDebug(fmt::format("    CLIP_DISTANCE0 = {}\n", CapToStr(GL_CLIP_DISTANCE0)));
    PrintDebug(fmt::format("    CLIP_DISTANCE1 = {}\n", CapToStr(GL_CLIP_DISTANCE1)));
    PrintDebug(fmt::format("    CLIP_DISTANCE2 = {}\n", CapToStr(GL_CLIP_DISTANCE2)));
    PrintDebug(fmt::format("    CLIP_DISTANCE3 = {}\n", CapToStr(GL_CLIP_DISTANCE3)));
    PrintDebug(fmt::format("    CLIP_DISTANCE4 = {}\n", CapToStr(GL_CLIP_DISTANCE4)));
    PrintDebug(fmt::format("    CLIP_DISTANCE5 = {}\n", CapToStr(GL_CLIP_DISTANCE5)));
    PrintDebug(fmt::format("    COLOR_LOGIC_OP = {}\n", CapToStr(GL_COLOR_LOGIC_OP)));
    PrintDebug(fmt::format("    CULL_FACE      = {}\n", CapToStr(GL_CULL_FACE)));
    PrintDebug(fmt::format("    DEBUG_OUTPUT   = {}\n", CapToStr(GL_DEBUG_OUTPUT)));
    PrintDebug(fmt::format("    DEBUG_OUTPUT_SYNCHRONOUS = {}\n", CapToStr(GL_DEBUG_OUTPUT_SYNCHRONOUS)));
    PrintDebug(fmt::format("    DEPTH_CLAMP = {}\n", CapToStr(GL_DEPTH_CLAMP)));
    PrintDebug(fmt::format("    DEPTH_TEST = {}\n", CapToStr(GL_DEPTH_TEST)));
    PrintDebug(fmt::format("    DITHER = {}\n", CapToStr(GL_DITHER)));
    PrintDebug(fmt::format("    FRAMEBUFFER_SRGB = {}\n", CapToStr(GL_FRAMEBUFFER_SRGB)));
    PrintDebug(fmt::format("    LINE_SMOOTH = {}\n", CapToStr(GL_LINE_SMOOTH)));
    PrintDebug(fmt::format("    MULTISAMPLE = {}\n", CapToStr(GL_MULTISAMPLE)));
    PrintDebug(fmt::format("    POLYGON_OFFSET_FILL = {}\n", CapToStr(GL_POLYGON_OFFSET_FILL)));
    PrintDebug(fmt::format("    POLYGON_OFFSET_LINE = {}\n", CapToStr(GL_POLYGON_OFFSET_LINE)));
    PrintDebug(fmt::format("    POLYGON_OFFSET_POINT = {}\n", CapToStr(GL_POLYGON_OFFSET_POINT)));
    PrintDebug(fmt::format("    POLYGON_SMOOTH = {}\n", CapToStr(GL_POLYGON_SMOOTH)));
    PrintDebug(fmt::format("    PRIMITIVE_RESTART = {}\n", CapToStr(GL_PRIMITIVE_RESTART)));
    PrintDebug(fmt::format("    PRIMITIVE_RESTART_FIXED_INDEX = {}\n", CapToStr(GL_PRIMITIVE_RESTART_FIXED_INDEX)));
    PrintDebug(fmt::format("    RASTERIZER_DISCARD = {}\n", CapToStr(GL_RASTERIZER_DISCARD)));
    PrintDebug(fmt::format("    SAMPLE_ALPHA_TO_COVERAGE = {}\n", CapToStr(GL_SAMPLE_ALPHA_TO_COVERAGE)));
    PrintDebug(fmt::format("    SAMPLE_ALPHA_TO_ONE = {}\n", CapToStr(GL_SAMPLE_ALPHA_TO_ONE)));
    PrintDebug(fmt::format("    SAMPLE_COVERAGE = {}\n", CapToStr(GL_SAMPLE_COVERAGE)));
    PrintDebug(fmt::format("    SAMPLE_SHADING = {}\n", CapToStr(GL_SAMPLE_SHADING)));
    PrintDebug(fmt::format("    SAMPLE_MASK = {}\n", CapToStr(GL_SAMPLE_MASK)));
    PrintDebug(fmt::format("    SCISSOR_TEST = {}\n", CapToStr(GL_SCISSOR_TEST)));
    PrintDebug(fmt::format("    STENCIL_TEST = {}\n", CapToStr(GL_STENCIL_TEST)));
    PrintDebug(fmt::format("    TEXTURE_CUBE_MAP_SEAMLESS = {}\n", CapToStr(GL_TEXTURE_CUBE_MAP_SEAMLESS)));
    PrintDebug(fmt::format("    PROGRAM_POINT_SIZE = {}\n", CapToStr(GL_PROGRAM_POINT_SIZE)));

    // Most flags are boring. Only print all of theme if we really want.
    if (details) {
    }
}


// Convert a Lua error value to a string.
std::string LuaErrorToString(int val)
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
    int got_stats = stat(RESOURCE_PATH.c_str(), &info);
    if (got_stats != 0) {
        return false;
    }

    int dir_flag = (info.st_mode & S_IFDIR);
    return (dir_flag == S_IFDIR);
}


// See if a file exists.
bool IsResource(const std::string &fname) {
    boost::filesystem::path my_path(fname);
    if (!my_path.is_complete()) {
        my_path = boost::filesystem::path(RESOURCE_PATH) / fname;
    }

    std::ifstream fs(my_path.string());
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
    boost::filesystem::path my_path(fname);
    if (!my_path.is_complete()) {
        my_path = boost::filesystem::path(RESOURCE_PATH) / fname;
    }

    if (!boost::filesystem::is_regular_file((my_path))) {
        PrintDebug(fmt::format("Text resource {} does not exist!\n", my_path.string()));
        assert(false);
        return false;
    }

    std::ifstream fs(my_path.string());
    if (!fs.is_open()) {
        PrintDebug(fmt::format("Count not open text resource {} for reading!\n", my_path.string()));
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
    boost::filesystem::path my_path(fname);
    if (!my_path.is_complete()) {
        my_path = boost::filesystem::path(RESOURCE_PATH) / fname;
    }

    if (!boost::filesystem::is_regular_file((my_path))) {
        PrintDebug(fmt::format("Font resource {} does not exist!\n", my_path.string()));
        assert(false);
        return false;
    }

    if (!pOut->loadFromFile(my_path.string())) {
        PrintDebug(fmt::format("Could not open font resource {}.\n", my_path.string()));
        assert(false);
        return false;
    }

    return true;
}


// Load an image resource.
// TODO: FIX THIS FILESYSTEM SHIT.
bool ReadImageResource(sf::Image *pOut, const std::string &fname)
{
    boost::filesystem::path my_path(fname);
    if (!my_path.is_complete()) {
        my_path = boost::filesystem::path(RESOURCE_PATH) / fname;
    }

    if (!boost::filesystem::is_regular_file((my_path))) {
        PrintDebug(fmt::format("Texture resource {} does not exist!\n", my_path.string()));
        assert(false);
        return false;
    }

    // Load the image.
    if (!pOut->loadFromFile(my_path.string())) {
        PrintDebug(fmt::format("Could not load texture resource {}.\n", my_path.string()));
        assert(false);
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

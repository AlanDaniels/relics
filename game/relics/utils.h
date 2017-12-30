
#pragma once

#include "my_math.h"

// A breakpoint to be triggered just once, when we hit the spacebar.
extern bool MAGIC_BREAKPOINT;

// Global settings.
const int FRAME_DELTA_MSECS = 25;

// The world units are centimeters.
const GLfloat BLOCK_SCALE = 100.0f;

// Using the standard size of 32 by 32 by 256 for now.
// The "X" width and the "Z" depth must be identical, or our logic would get
// ridiculously complicated, but our "Y" height can be whatever we wish.
const int CHUNK_WIDTH  = 32;
const int CHUNK_HEIGHT = 256;

const GLfloat NUDGE_AMOUNT = BLOCK_SCALE / 100.0f;


// Some C++ incantations to see if a future is ready yet.
template<typename R>
bool IsFutureReady(std::future<R> const& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}


// Useful general-purpose enums and functions.
enum class FaceType
{
    NONE   = 0,
    SOUTH  = 1,
    NORTH  = 2,
    WEST   = 3,
    EAST   = 4,
    TOP    = 5,
    BOTTOM = 6
};


enum class EdgeType
{
    NONE  = 0,
    UPPER = 1,
    LOWER = 2,
    LEFT  = 3,
    RIGHT = 4
};


enum class CornerType
{
    NONE        = 0,
    LOWER_LEFT  = 1,
    LOWER_RIGHT = 2,
    UPPER_LEFT  = 3,
    UPPER_RIGHT = 4
};


std::string FaceTypeToString(FaceType face_type);
std::string EdgeTypeToString(EdgeType edge_type);
std::string CornerTypeToString(CornerType corner_type);

GLfloat GridToWorld(int grid);

MyPlane GetSouthGridPlane(int grid_z);
MyPlane GetNorthGridPlane(int grid_z);
MyPlane GetWestGridPlane(int grid_x);
MyPlane GetEastGridPlane(int grid_x);
MyPlane GetTopGridPlane(int grid_y);
MyPlane GetBottomGridPlane(int grid_y);

void  PrintOpenglEnabledState(const char *title, bool details);

const char *LuaErrorToString(int error);

extern const char *RESOURCE_PATH;
bool DoesResourcePathExist();
bool IsResource(const std::string &fname);
std::string ReadTextResource(const std::string &fname);
bool ReadFontResource(sf::Font *pOut, const std::string &fname);
bool ReadImageResource(sf::Image *pOut, const std::string &fname);

int RoundUpInt(int val, int mult);
int RoundDownInt(int val, int mult);

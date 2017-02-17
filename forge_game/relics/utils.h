
#pragma once

#include "my_math.h"


// Global settings.
const int FRAME_DELTA_MSECS = 25;

// TEMP: Trying the standard size of 32 for now.
const int CHUNK_WIDTH_X  = 32;
const int CHUNK_DEPTH_Z  = 32;
const int CHUNK_HEIGHT_Y = 256;

const float BLOCK_SCALE = 100.0f;

const float NUDGE_AMOUNT = BLOCK_SCALE / 100.0f;


// Some C++ incantations to see if a future is ready yet.
template<typename R>
bool IsFutureReady(std::future<R> const& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}


// Useful general-purpose enums and functions.
enum FaceEnum
{
    FACE_NONE,
    FACE_SOUTH,
    FACE_NORTH,
    FACE_WEST,
    FACE_EAST,
    FACE_TOP,
    FACE_BOTTOM,
};


enum EdgeEnum
{
    EDGE_NONE,
    EDGE_UPPER,
    EDGE_LOWER,
    EDGE_LEFT,
    EDGE_RIGHT
};


enum CornerEnum
{
    CORNER_NONE,
    CORNER_LOWER_LEFT,
    CORNER_LOWER_RIGHT,
    CORNER_UPPER_LEFT,
    CORNER_UPPER_RIGHT
};


std::string FaceEnumToString(FaceEnum face);
std::string EdgeEnumToString(EdgeEnum edge);
std::string CornerEnumToString(CornerEnum corner);

GLfloat GridToWorld(int grid);

MyPlane GetSouthGridPlane(int grid_z);
MyPlane GetNorthGridPlane(int grid_z);
MyPlane GetWestGridPlane(int grid_x);
MyPlane GetEastGridPlane(int grid_x);
MyPlane GetTopGridPlane(int grid_y);
MyPlane GetBottomGridPlane(int grid_y);

void  PrintDebug(const char *format, ...);
void  PrintTheImpossible(const char *fname, int line_num, int value);
void  PrintOpenglEnabledState(const char *title, bool details);

const char *LuaErrorToString(int error);

bool IsResource(const std::string &fname);
std::string ReadTextResource(const std::string &fname);
bool ReadFontResource(sf::Font *pOut, const std::string &fname);
bool ReadImageResource(sf::Image *pOut, const std::string &fname);

int RoundUpInt(int val, int mult);
int RoundDownInt(int val, int mult);

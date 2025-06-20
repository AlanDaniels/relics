
#include "stdafx.h"
#include "physics.h"

#include "chunk.h"
#include "common_util.h"
#include "config.h"
#include "game_world.h"
#include "heads_up_display.h"
#include "player.h"


// Tweak things by one centimeter to avoid edge cases.
static const GLfloat TWEAK = 1.0f;

bool IsWorldBlockFilled(const GameWorld &world, const GlobalGrid &coord);
int  FloorToBlock(GLfloat val);
int  CeilToBlock(GLfloat val);

bool CollisionCheckDown (Player &player, int msec);
bool CollisionCheckUp   (Player &player, int msec);
bool CollisionCheckNorth(Player &player, int msec);
bool CollisionCheckSouth(Player &player, int msec);
bool CollisionCheckEast (Player &player, int msec);
bool CollisionCheckWest (Player &player, int msec);


// Run collision tests in all six directions.
void PlayerCollisionTest(Player &player, int msec)
{
    // If "noclip" is on, don't bother.
    if (GetConfig().debug.noclip) {
        return;
    }

    std::string msg = "";

    // Start with up/down collisions.
    if (CollisionCheckDown(player, msec)) {
        msg += (msg == "") ? "Down" : " Down";
    }
    if (CollisionCheckUp(player, msec)) {
        msg += (msg == "") ? "Up" : " Up";
    }

    // For north/south (Z) vs east/west (X), figure out which
    // axis is more predominant, and check those first. This
    // way, we don't have to shrink BBoxes quite so much, to
    // avoid "snagging" into walls.
    MyVec4 vert = player.getHorzMotion();
    GLfloat x = vert.x();
    GLfloat z = vert.z();
    bool z_is_bigger = ::abs(z) > ::abs(x);

    // Z is bigger, so North/South first.
    if (z_is_bigger) {
        if (CollisionCheckNorth(player, msec)) {
            msg += (msg == "") ? "North" : " North";
        }
        if (CollisionCheckSouth(player, msec)) {
            msg += (msg == "") ? "South" : " South";
        }
        if (CollisionCheckEast(player, msec)) {
            msg += (msg == "") ? "East" : " East";
        }
        if (CollisionCheckWest(player, msec)) {
            msg += (msg == "") ? "West" : " West";
        }
    }

    // X is bigger, so East/West first.
    else {
        if (CollisionCheckEast(player, msec)) {
            msg += (msg == "") ? "East" : " East";
        }
        if (CollisionCheckWest(player, msec)) {
            msg += (msg == "") ? "West" : " West";
        }
        if (CollisionCheckNorth(player, msec)) {
            msg += (msg == "") ? "North" : " North";
        }
        if (CollisionCheckSouth(player, msec)) {
            msg += (msg == "") ? "South" : " South";
        }
    }

    // All done.
    SetHudCollisionLine(msg);
}


// Return true if a block within the world is filled. We're going to rely
// on locality of reference to hope that this doesn't hammer memory too bad.
bool IsWorldBlockFilled(const GameWorld &world, const GlobalGrid &coord)
{
    if (!coord.isWithinWorld()) {
        return false;
    }

    ChunkOrigin origin = GlobalGridToChunkOrigin(coord);
    LocalGrid local_coord = GlobalGridToLocal(coord, origin);

    const Chunk *chunk = world.getChunk(origin);
    if (chunk == nullptr) {
        return false;
    }
    else {
        BlockType bt = chunk->getBlockType(local_coord);
        bool result = IsBlockTypeFilled(bt);
        return result;
    }
}


// Floor exactly to our block scale.
int FloorToBlock(GLfloat val)
{
    int block_scale = static_cast<int>(BLOCK_SCALE);
    int int_val = static_cast<int>(val);
    int rounded = RoundDownInt(int_val, block_scale);
    int result  = rounded / block_scale;
    return result;
}


// Ceiling exactly to our block scale.
int CeilToBlock(GLfloat val)
{
    int block_scale = static_cast<int>(BLOCK_SCALE);
    int int_val = static_cast<int>(val);
    int rounded = RoundUpInt(int_val, block_scale);
    int result  = rounded / block_scale;
    return result;
}


// Check a collisioin against the floor (-Y direction).
bool CollisionCheckDown(Player &player, int msec)
{
    GLfloat y_motion = player.getVertMotion().y();
    GLfloat x_motion = player.getHorzMotion().x();
    GLfloat z_motion = player.getHorzMotion().z();

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // If the player has no -Y motion, don't bother.
    if (y_motion >= 0) {
        return false;
    }

    // For checking to fall through the floor, shrink our bounding box
    // so that we don't "stick" to walls and be able to climb them.
    GLfloat shrink_x = (::abs(x_motion) * msec) + TWEAK;
    GLfloat shrink_z = (::abs(z_motion) * msec) + TWEAK;

    // Establish the boundaries of blocks to look at.
    int min_x = FloorToBlock(bbox.minX() + shrink_x);
    int max_x = CeilToBlock (bbox.maxX() - shrink_x);
    int min_z = FloorToBlock(bbox.minZ() + shrink_z);
    int max_z = CeilToBlock (bbox.maxZ() - shrink_z);

    int constant_y = FloorToBlock(bbox.minY());

    bool collision = false;
    for     (int x = min_x; x < max_x; x++) {
        for (int z = min_z; z < max_z; z++) {
            GlobalGrid coord(x, constant_y, z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collide, plant the player at that spot. The is the
    // only place where we don't tweak the offset when corrrecting
    // where the player ends up, since otherwise the constant
    // effect of gravity would cause visual jitter.
    if (collision) {
        player.setOnSolidGround(true);

        MyVec4  pos = player.getPlayerPos();
        GLfloat the_floor = (constant_y + 1) * BLOCK_SCALE;

        MyVec4  new_pos(pos.x(), the_floor, pos.z());
        player.setPlayerPos(new_pos);
        return true;
    }
    else {
        player.setOnSolidGround(false);
    }

    return false;
}


// Check a collisioin against the ceiling (+Y direction). Really rare.
bool CollisionCheckUp(Player &player, int msec)
{
    GLfloat y_motion = player.getVertMotion().y();
    GLfloat x_motion = player.getHorzMotion().x();
    GLfloat z_motion = player.getHorzMotion().z();

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // If the player has no +Y motion, don't bother.
    if (y_motion <= 0) {
        return false;
    }

    // For checking against the ceiling, shrink our bounding box
    // so that we don't "stick" to walls and be able to climb them.
    GLfloat shrink_x = (::abs(x_motion) * msec) + TWEAK;
    GLfloat shrink_z = (::abs(z_motion) * msec) + TWEAK;

    // Establish the boundaries of blocks to look at.
    int min_x = FloorToBlock(bbox.minX() + shrink_x);
    int max_x = CeilToBlock (bbox.maxX() - shrink_x);
    int min_z = FloorToBlock(bbox.minZ() + shrink_z);
    int max_z = CeilToBlock (bbox.maxZ() - shrink_z);

    int constant_y = FloorToBlock(bbox.maxY());

    bool collision = false;
    for     (int x = min_x; x < max_x; x++) {
        for (int z = min_z; z < max_z; z++) {
            GlobalGrid coord(x, constant_y, z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, prevent further motion in the +Y direction.
    // Also unique to here, bounce the player right back down.
    if (collision) {
        MyVec4  pos = player.getPlayerPos();
        GLfloat the_ceiling = constant_y * BLOCK_SCALE;
        GLfloat offset = bbox.getHeight() + TWEAK;

        MyVec4  new_pos(pos.x(), the_ceiling - offset, pos.z());
        player.setPlayerPos(new_pos);

        player.bounceOffCeiling();
        return true;
    }

    return false;
}


// Check a collisioin against what's in the +Z direction.
bool CollisionCheckNorth(Player &player, int msec)
{
    GLfloat x_motion = player.getHorzMotion().x();
    GLfloat z_motion = player.getHorzMotion().z();

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // If the player has no +Z motion, don't bother.
    if (z_motion <= 0) {
        return false;
    }

    // Shrink the bounding box so that it doesn't scrape the
    // floor and ceilinng, and vertically a bit to avoid edge cases.
    GLfloat shrink_x = (::abs(x_motion) * msec) + TWEAK;

    // Establish the boundaries of blocks to look at.
    int min_x = FloorToBlock(bbox.minX() + shrink_x);
    int max_x = CeilToBlock (bbox.maxX() - shrink_x);
    int min_y = FloorToBlock(bbox.minY() + TWEAK);
    int max_y = CeilToBlock (bbox.maxY() - TWEAK);

    int constant_z = FloorToBlock(bbox.maxZ());

    bool collision = false;
    for     (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            GlobalGrid coord(x, y, constant_z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, prevent further motion in the +Z direction.
    if (collision) {
        MyVec4  pos = player.getPlayerPos();
        GLfloat north_wall = constant_z * BLOCK_SCALE;
        GLfloat offset = (bbox.getDepth() / 2.0f) + TWEAK;

        MyVec4 new_pos(pos.x(), pos.y(), north_wall - offset);
        player.setPlayerPos(new_pos);
        return true;
    }

    return false;
}


// Check a collisioin against what's in the -Z direction.
bool CollisionCheckSouth(Player &player, int msec)
{
    GLfloat x_motion = player.getHorzMotion().x();
    GLfloat z_motion = player.getHorzMotion().z();

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // If the player has no -Z motion, don't bother.
    if (z_motion >= 0) {
        return false;
    }

    // Shrink the bounding box so that it doesn't scrape the
    // floor and ceilinng, and vertically a bit to avoid edge cases.
    GLfloat shrink_x = (::abs(x_motion) * msec) + TWEAK;

    // Establish the boundaries of blocks to look at.
    int min_x = FloorToBlock(bbox.minX() + shrink_x);
    int max_x = CeilToBlock (bbox.maxX() - shrink_x);
    int min_y = FloorToBlock(bbox.minY() + TWEAK);
    int max_y = CeilToBlock (bbox.maxY() - TWEAK);

    int constant_z = FloorToBlock(bbox.minZ());

    bool collision = false;
    for     (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            GlobalGrid coord(x, y, constant_z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, prevent further motion in the -Z direction.
    if (collision) {
        MyVec4  pos = player.getPlayerPos();
        GLfloat south_wall = (constant_z + 1) * BLOCK_SCALE;
        GLfloat offset = (bbox.getDepth() / 2.0f) + TWEAK;

        MyVec4  new_pos(pos.x(), pos.y(), south_wall + offset);
        player.setPlayerPos(new_pos);
        return true;
    }

    return false;
}


// Check a collisioin against what's in the +X direction.
bool CollisionCheckEast(Player &player, int msec)
{
    GLfloat x_motion = player.getHorzMotion().x();
    GLfloat z_motion = player.getHorzMotion().z();

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // If the player has no +X motion, don't bother.
    if (x_motion <= 0) {
        return false;
    }

    // Shrink the bounding box so that it doesn't scrape the
    // floor and ceilinng, and vertically a bit to avoid edge cases.
    GLfloat shrink_z = (::abs(z_motion) * msec) + TWEAK;

    // Establish the boundaries of blocks to look at.
    int min_y = FloorToBlock(bbox.minY() + TWEAK);
    int max_y = CeilToBlock (bbox.maxY() - TWEAK);
    int min_z = FloorToBlock(bbox.minZ() + shrink_z);
    int max_z = CeilToBlock (bbox.maxZ() - shrink_z);

    int constant_x = FloorToBlock(bbox.maxX());

    bool collision = false;
    for     (int y = min_y; y < max_y; y++) {
        for (int z = min_z; z < max_z; z++) {
            GlobalGrid coord(constant_x, y, z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, prevent further motion in the +X direction.
    if (collision) {
        MyVec4  pos = player.getPlayerPos();
        GLfloat east_wall = constant_x * BLOCK_SCALE;
        GLfloat offset = (bbox.getWidth() / 2.0f) + TWEAK;

        MyVec4 new_pos(east_wall - offset, pos.y(), pos.z());
        player.setPlayerPos(new_pos);
        return true;
    }

    return false;
}


// Check a collisioin against what's in the -X direction.
bool CollisionCheckWest(Player &player, int msec)
{
    GLfloat x_motion = player.getHorzMotion().x();
    GLfloat z_motion = player.getHorzMotion().z();

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // If the player has no -X motion, don't bother.
    if (x_motion >= 0) {
        return false;
    }

    // Shrink the bounding box so that it doesn't scrape the
    // floor and ceilinng, and vertically a bit to avoid edge cases.
    GLfloat shrink_z = (::abs(z_motion) * msec) + TWEAK;

    // Establish the boundaries of blocks to look at.
    int min_y = FloorToBlock(bbox.minY() + TWEAK);
    int max_y = CeilToBlock (bbox.maxY() - TWEAK);
    int min_z = FloorToBlock(bbox.minZ() + shrink_z);
    int max_z = CeilToBlock (bbox.maxZ() - shrink_z);

    int constant_x = FloorToBlock(bbox.minX());

    bool collision = false;
    for     (int y = min_y; y < max_y; y++) {
        for (int z = min_z; z < max_z; z++) {
            GlobalGrid coord(constant_x, y, z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, prevent further motion in the -X direction.
    if (collision) {
        MyVec4  pos = player.getPlayerPos();
        GLfloat west_wall = (constant_x + 1) * BLOCK_SCALE;
        GLfloat offset = (bbox.getWidth() / 2.0f) + TWEAK;

        MyVec4 new_pos(west_wall + offset, pos.y(), pos.z());
        player.setPlayerPos(new_pos);
        return true;
    }

    return false;
}

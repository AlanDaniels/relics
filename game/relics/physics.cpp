
#include "stdafx.h"
#include "physics.h"

#include "chunk.h"
#include "common_util.h"
#include "config.h"
#include "game_world.h"
#include "heads_up_display.h"
#include "player.h"


bool IsWorldBlockFilled(const GameWorld &world, const GlobalGrid &coord);
int  FloorToBlock(GLfloat val);
int  CeilToBlock(GLfloat val);

bool CollisionCheckDown(Player &player);
bool CollisionCheckNorth(Player &player);


// This will probably be the most difficult thing to write so farr.
// To start, just work on keeping the player from falling through
// the floor, using nothing but simple plane tests.
void PlayerCollisionTest(Player &player)
{
    // If "noclip" is on, don't bother.
    if (GetConfig().debug.noclip) {
        return;
    }

    CollisionCheckDown(player);
    CollisionCheckNorth(player);
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

    const Chunk *chunk = world.getRequiredChunk(origin);
    BlockType bt = chunk->getBlockType(local_coord);
    bool result = IsBlockTypeFilled(bt);
    return result;
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
bool CollisionCheckDown(Player &player)
{
    // If the player doesn't have any -Z motion, don't bother.
    if (player.getHorzMotion().y() >= 0) {
        return false;
    }

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // For checking to fall through the floor, shrink our bounding box
    // so that we don't "stick" to walls and be able to climb them.
    // This area needs to be smaller than the walls of the bounding
    // box in order for this to work.
    GLfloat shrink = 2.0f;

    // Establish the boundaries of blocks to look at.
    int min_x = FloorToBlock(bbox.minX() + shrink);
    int max_x = CeilToBlock (bbox.maxX() - shrink);
    int min_z = FloorToBlock(bbox.minZ() + shrink);
    int max_z = CeilToBlock (bbox.maxZ() - shrink);

    int y = FloorToBlock(bbox.minY());

    bool collision = false;
    for     (int x = min_x; x < max_x; x++) {
        for (int z = min_z; z < max_z; z++) {
            GlobalGrid coord(x, y, z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, plant the player at that spot.
    player.setOnSolidGround(collision);
    if (collision) {
        MyVec4  current_pos = player.getPlayerPos();
        GLfloat y_wall = (y + 1) * BLOCK_SCALE;
        MyVec4  new_pos(current_pos.x(), y_wall, current_pos.z());
        player.setPlayerPos(new_pos);
        return true;
    }

    return false;
}


// Check a collisioin against what's in the +Z direction.
bool CollisionCheckNorth(Player &player)
{
    // If the player doesn't have any +Z motion, don't bother.
    if (player.getVertMotion().z() <= 0) {
        SetDebugLine("");
        return false;
    }

    const GameWorld     &world = player.getGameWorld();
    const MyBoundingBox &bbox  = player.getBoundingBox();

    // Shrink the bounding box a bit to avoid edge cases.
    GLfloat shrink = 1.0f;

    // Establish the boundaries of blocks to look at.
    int min_x = FloorToBlock(bbox.minX() + shrink);
    int max_x = CeilToBlock (bbox.maxX() - shrink);
    int min_y = FloorToBlock(bbox.minY() + shrink);
    int max_y = CeilToBlock (bbox.maxY() - shrink);

    int z = FloorToBlock(bbox.maxZ());

    bool collision = false;
    for     (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_y; y++) {
            GlobalGrid coord(x, y, z);
            if (IsWorldBlockFilled(world, coord)) {
                collision = true;
            }
        }
    }

    // If we collided, prevent the player from moving further in the +Z direction.
    // Add an extra "kick" outward to avoid edge cases.
    if (collision) {
        SetDebugLine("*** NORTH COLLISION ***");
        MyVec4  current_pos = player.getPlayerPos();
        GLfloat z_wall   = z * BLOCK_SCALE;
        GLfloat z_offset = (bbox.getDepth() / 2.0f) + shrink;
        MyVec4  new_pos(current_pos.x(), current_pos.y(), z_wall - z_offset);

        player.setPlayerPos(new_pos);
        return true;
    }

    return false;
}

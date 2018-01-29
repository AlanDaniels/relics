
#include "stdafx.h"
#include "physics.h"

#include "chunk.h"
#include "game_world.h"
#include "player.h"


// This will probably be the most difficult thing to write so farr.
// To start, just work on keeping the player from falling through
// the floor, using nothing but simple plane tests.
// TODO: Definitely work on this more later.
void PlayerCollisionTest(Player &player)
{
    const GameWorld &world = player.getGameWorld();

    const auto &bbox = player.getBoundingBox();

    GLfloat min_x = bbox.minX();
    GLfloat max_x = bbox.maxX();
    GLfloat     y = bbox.minY();
    GLfloat min_z = bbox.minZ();
    GLfloat max_z = bbox.maxZ();

    GlobalGrid SW = WorldPosToGlobalGrid(MyVec4(min_x, y, min_z), NudgeType::DOWN);
    GlobalGrid NE = WorldPosToGlobalGrid(MyVec4(max_x, y, max_z), NudgeType::DOWN);

    int grid_y = SW.y();

    bool collision = false;
    for     (int x = SW.x(); x <= NE.x(); x++) {
        for (int z = SW.z(); z <= NE.z(); z++) {
            GlobalGrid here(x, grid_y, z);
            if (here.isValid()) {
                ChunkOrigin origin = GlobalGridToChunkOrigin(here);
                LocalGrid local_here = GlobalGridToLocal(here, origin);

                const Chunk *chunk = world.getRequiredChunk(origin);
                BlockType bt = chunk->getBlockType(local_here);
                if (IsBlockTypeFilled(bt)) {
                    collision = true;
                }
            }
        }
    }

    // If we collided, plant the player at that spot.
    if (collision) {
        player.resetGravityVec();
        MyVec4  current_pos = player.getPlayerPos();
        GLfloat y_at_top_of_block = (grid_y + 1) * BLOCK_SCALE;
        MyVec4  new_pos(current_pos.x(), y_at_top_of_block, current_pos.z());
        player.setPlayerPos(new_pos);
    }
}
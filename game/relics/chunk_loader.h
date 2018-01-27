#pragma once

#include "stdafx.h"

#include "my_math.h"


class Chunk;
class GameWorld;
class ChunkOrigin;


// Find the player's start pos.
MyVec4 GetPlayerStartPos(const std::string &db_fname);

// Load a chunk.
std::unique_ptr<Chunk> LoadChunk(GameWorld &world, const ChunkOrigin &origin);

// Save a chunk.
void SaveChunk(GameWorld &world, std::unique_ptr<Chunk> chunk);
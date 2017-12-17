#pragma once

#include "stdafx.h"


class Chunk;
class GameWorld;
class ChunkOrigin;


// Load a chunk.
std::unique_ptr<Chunk> LoadChunk(GameWorld &world, const ChunkOrigin &origin);
void SaveChunk(std::unique_ptr<Chunk> chunk);
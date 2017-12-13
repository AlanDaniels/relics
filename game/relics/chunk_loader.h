#pragma once

#include "stdafx.h"


class Chunk;
class GameWorld;
class ChunkOrigin;


// Load a chunk.
Chunk *LoadChunk(GameWorld *pWorld, const ChunkOrigin &origin);

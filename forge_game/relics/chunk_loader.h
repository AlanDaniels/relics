#pragma once

#include "stdafx.h"


class Chunk;
class GameWorld;
class MyChunkOrigin;


// When we call "LoadChunkAsync" via std::async, we'll get back a future like this.
typedef std::future<Chunk *> t_chunk_future;


// Load a chunk. This should ideally be called via "std::async".
Chunk *LoadChunkAsync(const GameWorld *pWorld, const MyChunkOrigin &origin);

#pragma once

#include "my_math.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"
#include "utils.h"


class Chunk;


std::array<Vertex_PNT, 6> GetLandscapePatch_PNT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceType face);
std::array<Vertex_PT, 6> GetLandscapePatch_PT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceType face);
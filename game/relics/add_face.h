#pragma once

#include "my_math.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"


class Chunk;
enum  FaceEnum;


void AddFace_VertList_PNT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceEnum face,
    VertList_PNT *pOut);

void AddFace_VertList_PT(
    const Chunk &chunk, const LocalGrid &local_coord, FaceEnum face,
    VertList_PT *pOut);
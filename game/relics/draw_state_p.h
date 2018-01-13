#pragma once

#include "stdafx.h"
#include "draw_state_base.h"
#include "my_math.h"
#include "vert_list_base.h"


// A shader for dealing with data with one Position.

struct Vertex_P
{
    MyVec4 position;
};

static_assert(sizeof(Vertex_P) == 16, "Vertex_P should be 16 bytes.");


typedef VertList_Base<Vertex_P> VertList_P;


class DrawState_P : public DrawState_Base
{
public:
    DrawState_P(int uniform_texture_count) :
        DrawState_Base(uniform_texture_count) {}
    virtual ~DrawState_P() {}

    bool create(const DrawStateSettings &settings);
    bool render(const VertList_P &vert_list) const;

private:
    FORBID_DEFAULT_CTOR(DrawState_P)
    FORBID_COPYING(DrawState_P)
    FORBID_MOVING(DrawState_P)
};

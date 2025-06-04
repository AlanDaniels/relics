#pragma once

#include "stdafx.h"
#include "draw_state_base.h"
#include "vert_list_base.h"
#include "my_math.h"


// A shader for dealing with data with one Position, one Color, and one TexUV.


struct Vertex_PCT
{
    MyVec4  position;
    MyColor color;
    MyVec2  texuv;
};


static_assert(sizeof(Vertex_PCT) == 40, "Vertex_PCT should be 40 bytes.");


typedef VertList_Base<Vertex_PCT> VertList_PCT;


class DrawState_PCT : public DrawState_Base
{
public:
    DrawState_PCT(int uniform_texture_count) :
        DrawState_Base(uniform_texture_count) {}
    virtual ~DrawState_PCT() {}

    bool create(const DrawStateSettings &settings);
    bool render(const VertList_PCT &vert_list);

private:
    FORBID_DEFAULT_CTOR(DrawState_PCT)
    FORBID_COPYING(DrawState_PCT)
    FORBID_MOVING(DrawState_PCT)
};

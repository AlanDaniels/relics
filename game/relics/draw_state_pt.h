#pragma once

#include "stdafx.h"
#include "draw_state_base.h"
#include "my_math.h"
#include "vert_list_base.h"


// A shader for dealing with data with one Position, and one TexUV.

struct Vertex_PT
{
    Vertex_PT(const MyVec4 &arg_pos, const MyVec2 arg_texuv) :
        position(arg_pos), texuv(arg_texuv) {}

    MyVec4 position;
    MyVec2 texuv;
};

static_assert(sizeof(Vertex_PT) == 24, "Vertex_PT should be 24 bytes.");


typedef VertList_Base<Vertex_PT> VertList_PT;


class DrawState_PT : public DrawState_Base
{
public:
    DrawState_PT(int uniform_texture_count) :
        DrawState_Base(uniform_texture_count) {}
    virtual ~DrawState_PT() {}

    bool create(const DrawStateSettings &settings);
    bool render(const VertList_PT &vert_list);

private:
    DISALLOW_DEFAULT(DrawState_PT)
    DISALLOW_COPYING(DrawState_PT)
    DISALLOW_MOVING(DrawState_PT)
};

#pragma once

#include "stdafx.h"
#include "draw_state_base.h"
#include "vert_list_base.h"
#include "my_math.h"


// A shader for dealing with data with one Position, one Normal, and one TexUV.


struct Vertex_PNT
{
    Vertex_PNT(const MyVec4 &arg_pos, const MyVec4 &arg_normal, const MyVec2 &arg_texuv) :
        position(arg_pos), normal(arg_normal), texuv(arg_texuv) {}

    MyVec4 position;
    MyVec4 normal;
    MyVec2 texuv;
};


static_assert(sizeof(Vertex_PNT) == 40, "Vertex_PNT should be 40 bytes.");


typedef VertList_Base<Vertex_PNT> VertList_PNT;


class DrawState_PNT : public DrawState_Base
{
public:
    DrawState_PNT(int uniform_texture_count) :
        DrawState_Base(uniform_texture_count) {}
    virtual ~DrawState_PNT() {}

    bool create(const DrawStateSettings &settings);
    bool render(const VertList_PNT &vert_list);

private:
    FORBID_DEFAULT_CTOR(DrawState_PNT)
    FORBID_COPYING(DrawState_PNT)
    FORBID_MOVING(DrawState_PNT)
};

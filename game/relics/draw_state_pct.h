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
    // Forbid copying, and default ctor.
    DrawState_PCT() = delete;
    DrawState_PCT(const DrawState_PCT &that) = delete;
    void operator=(const DrawState_PCT &that) = delete;
};

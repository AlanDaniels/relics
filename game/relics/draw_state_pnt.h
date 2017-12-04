#pragma once

#include "stdafx.h"
#include "draw_state_base.h"
#include "vert_list_base.h"
#include "my_math.h"


// A shader for dealing with data with one Position, one Normal, and one TexUV.


struct Vertex_PNT
{
    MyVec4 position;
    MyVec4 normal;
    MyVec2 texuv;
};


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
    // Forbid copying, and default ctor.
    DrawState_PNT() = delete;
    DrawState_PNT(const DrawState_PNT &that) = delete;
    void operator=(const DrawState_PNT &that) = delete;
};

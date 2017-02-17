#pragma once

#include "stdafx.h"
#include "draw_state_base.h"
#include "my_math.h"
#include "vert_list_base.h"


// A shader for dealing with data with one Position, and one TexUV.

struct Vertex_PT
{
    MyVec4 position;
    MyVec2 texuv;
};


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
    // Forbid copying, and default ctor.
    DrawState_PT() = delete;
    DrawState_PT(const DrawState_PT &that) = delete;
    void operator=(const DrawState_PT &that) = delete;
};

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
    bool render(const VertList_P &vert_list);

private:
    // Disallow default ctor, copying, and moving.
    DrawState_P() = delete;
    DrawState_P(const DrawState_P &that) = delete;
    void operator=(const DrawState_P &that) = delete;
    DrawState_P(DrawState_P &&that) = delete;
    void operator=(DrawState_P &&that) = delete;
};

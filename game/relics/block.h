#pragma once

#include "stdafx.h"
#include "draw_state_pt.h"
#include "utils.h"


enum BlockContent
{
    CONTENT_AIR,
    CONTENT_GRASS,
    CONTENT_DIRT,
    CONTENT_STONE,
    CONTENT_BEDROCK,
};


bool IsContentEmpty(BlockContent content);


enum BlockSurface
{
    SURF_NONE,
    SURF_GRASS,
    SURF_DIRT,
    SURF_STONE,
    SURF_BEDROCK,
};


class Block
{
public:
    Block();

    BlockContent getContent() const { return m_content; }
    void setContent(BlockContent val) { m_content = val; }

    void clearExposureFlags();

    bool getExposure(FaceEnum face) const;
    void setExposure(FaceEnum face, bool val);

    bool isEmpty()  const { return  IsContentEmpty(m_content); }
    bool isFilled() const { return !IsContentEmpty(m_content); }

private:
    // Disallow copying.
    Block(const Block &that) = delete;
    void operator=(const Block &that) = delete;

    BlockContent m_content;
    bool m_west_exposed;
    bool m_east_exposed;
    bool m_south_exposed;
    bool m_north_exposed;
    bool m_top_exposed;
    bool m_bottom_exposed;
};

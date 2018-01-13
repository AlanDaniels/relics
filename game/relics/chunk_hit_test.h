#pragma once

#include "stdafx.h"

#include "chunk.h"
#include "draw_state_pt.h"


// All the details about a hit test.
class ChunkHitTestDetail
{
public:
    // Default ctor. Set the distance to the worst possible value.
    ChunkHitTestDetail() :
        m_face(FaceType::NONE),
        m_dist(FLT_MAX) {}
        
    ChunkHitTestDetail(
        const ChunkOrigin &origin,
        const GlobalGrid &coord,
        FaceType face, 
        const MyVec4 &impact, 
        GLfloat dist) :
        m_chunk_origin(origin),
        m_global_coord(coord),
        m_face(face),
        m_impact(impact),
        m_dist(dist) {}

    DEFAULT_MOVING(ChunkHitTestDetail)

    const ChunkOrigin &getChunkOrigin() const { return m_chunk_origin; }
    const GlobalGrid  &getGlobalCoord() const { return m_global_coord; }
    FaceType getFace() const { return m_face; }
    const MyVec4 &getImpact() const { return m_impact; }
    GLfloat getDist() const { return m_dist; }

    std::string toDescr() const;

private:
    FORBID_COPYING(ChunkHitTestDetail)

    // Private data
    ChunkOrigin m_chunk_origin;
    GlobalGrid  m_global_coord;
    FaceType    m_face;
    MyVec4      m_impact;
    GLfloat     m_dist;
};


// The two functions that do the work.
bool DoChunkHitTest(const Chunk &chunk, const MyRay &eye_ray, ChunkHitTestDetail *pOut);
void ChunkHitTestToQuad(const Chunk &chunk, const ChunkHitTestDetail &details, VertList_PT *pOut);
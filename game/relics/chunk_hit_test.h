#pragma once

#include "chunk.h"
#include "draw_state_pt.h"


// All the details about a hit test.
class ChunkHitTestDetail
{
public:
    // The default ctor, which we should only call in one place.
    // Note that we set the distance to the worst possible value.
    ChunkHitTestDetail() :
        m_face(FACE_NONE),
        m_dist(FLT_MAX) {}
        
    ChunkHitTestDetail(
        const MyChunkOrigin &origin, const MyGridCoord &coord,
        FaceEnum face, const MyVec4 &impact, GLfloat dist) :
        m_chunk_origin(origin),
        m_global_coord(coord),
        m_face(face),
        m_impact(impact),
        m_dist(dist) {}

    ChunkHitTestDetail(const ChunkHitTestDetail &that) :
        m_chunk_origin(that.m_chunk_origin),
        m_global_coord(that.m_global_coord),
        m_face(that.m_face),
        m_impact(that.m_impact),
        m_dist(that.m_dist) {}

    const ChunkHitTestDetail &operator=(const ChunkHitTestDetail &that);

    const MyChunkOrigin &getChunkOrigin() const { return m_chunk_origin; }
    const MyGridCoord &getGlobalCoord() const { return m_global_coord; }
    FaceEnum getFace() const { return m_face; }
    const MyVec4 &getImpact() const { return m_impact; }
    GLfloat getDist() const { return m_dist; }

    std::string toDescription() const;

private:
    MyChunkOrigin m_chunk_origin;
    MyGridCoord   m_global_coord;
    FaceEnum      m_face;
    MyVec4        m_impact;
    GLfloat       m_dist;
};


// The two functions that do the work.
bool DoChunkHitTest(const Chunk &chunk, const MyRay &eye_ray, ChunkHitTestDetail *pOut);
void ChunkHitTestToQuad(const Chunk &chunk, const ChunkHitTestDetail &details, VertList_PT *pOut);
#pragma once

#include "stdafx.h"
#include "draw_cubemap_texture.h"
#include "draw_state_p.h"
#include "draw_texture.h"
#include "game_world.h"


struct RenderStats
{
    RenderStats() :
        chunks_considered(0),
        chunks_rendered(0),
        state_changes(0),
        triangle_count(0) {}

    int chunks_considered;
    int chunks_rendered;
    int state_changes;
    int triangle_count;
};


// For rendering the game world. Separating logic from presentation.
class Renderer
{
public:
    Renderer(const sf::Window &window, const GameWorld &world) :
        m_window(window),
        m_world(world) {}

    bool init();
    RenderStats renderWorld();

private:
    FORBID_DEFAULT_CTOR(Renderer)
    FORBID_COPYING(Renderer)
    FORBID_MOVING(Renderer)

    // Private methods.
    void rebuildUniformMatrices();
    void buildSkyboxVertList();

    std::vector<const Chunk *> getChunksToRender(RenderStats *pOut_stats);

    void renderSkybox(RenderStats *pOut_stats);

    void renderLandscapeList(
        SurfaceType surf, 
        const std::vector<const Chunk *> &chunk_list,
        const DrawTexture &tex,
        RenderStats *pOut_stats);

    void renderWFObjects(std::vector<const Chunk *> &chunk_list, RenderStats *pOut_stats);

    void renderHitTest(RenderStats *pOut_stats);

    // Private data.
    const sf::Window &m_window;
    const GameWorld  &m_world;

    MyMatrix4by4 m_frustum_matrix;
    MyMatrix4by4 m_frustum_rotate_matrix;

    VertList_P m_skybox_vert_list;
};

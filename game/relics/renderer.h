#pragma once

#include "stdafx.h"
#include "draw_cubemap_texture.h"
#include "draw_state_p.h"
#include "draw_state_pt.h"
#include "draw_state_pnt.h"
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
    // Forbid copying, and default ctor.
    Renderer() = delete;
    Renderer(const Renderer &that) = delete;
    void operator=(const Renderer &that) = delete;

    void rebuildUniformMatrices();
    void buildSkyboxVertList();
    void loadTextures();
    bool loadShaders();

    void getChunksToRender(std::vector<const Chunk *> *pOut_chunk_list, RenderStats *pOut_stats);

    void renderSkybox(RenderStats *pOut_stats);

    void renderLandscapeList(
        BlockSurface surf, const std::vector<const Chunk *> &chunk_list,
        const DrawTexture &tex, RenderStats *pOut_stats);

    void renderHitTest(RenderStats *pOut_stats);

    const sf::Window &m_window;
    const GameWorld &m_world;

    MyMatrix4by4 m_frustum_matrix;
    MyMatrix4by4 m_frustum_rotate_matrix;

    std::unique_ptr<DrawTexture> m_grass_tex;
    std::unique_ptr<DrawTexture> m_dirt_tex;
    std::unique_ptr<DrawTexture> m_stone_tex;
    std::unique_ptr<DrawTexture> m_bedrock_tex;

    std::unique_ptr<DrawTexture> m_hit_test_tex;

    std::unique_ptr<DrawCubemapTexture> m_skybox_tex;

    std::unique_ptr<DrawState_P> m_skybox_DS;

    std::unique_ptr<DrawState_PNT> m_landscape_DS;
    std::unique_ptr<DrawState_PT>  m_hit_test_DS;

    VertList_P m_skybox_vert_list;
};

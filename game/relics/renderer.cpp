
#include "stdafx.h"
#include "common_util.h"
#include "renderer.h"

#include "config.h"
#include "draw_state_p.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"
#include "draw_texture.h"


bool Renderer::init()
{
    buildSkyboxVertList();
    loadTextures();
    if (!loadShaders()) {
        return false;
    }

    return true;
}


// Build our standard model-view matrix.
void Renderer::rebuildUniformMatrices()
{
    GLfloat fov = GetConfig().render.field_of_view;

    sf::Vector2u dims = m_window.getSize();
    GLfloat screen_width  = static_cast<GLfloat>(dims.x);
    GLfloat screen_height = static_cast<GLfloat>(dims.y);

    GLfloat angle_of_view = DegreesToRadians(fov / 2.0f);
    GLfloat aspect_ratio  = screen_width / screen_height;
    GLfloat near_plane_cm = GetConfig().render.getNearPlaneCm();
    GLfloat far_plane_cm  = GetConfig().render.getFarPlaneCm();

    // Just the frustum projection.
    m_frustum_matrix = MyMatrix4by4::Frustum(angle_of_view, aspect_ratio, near_plane_cm, far_plane_cm);

    // That, plus the rotations.
    GLfloat camera_pitch = m_world.getCameraPitch();
    GLfloat camera_yaw   = m_world.getCameraYaw();

    MyMatrix4by4 rotate_x = MyMatrix4by4::RotateX(-camera_pitch);
    MyMatrix4by4 rotate_y = MyMatrix4by4::RotateY(camera_yaw);
    m_frustum_rotate_matrix = rotate_y.times(rotate_x).times(m_frustum_matrix);
}


// Build the skybox vert list.
void Renderer::buildSkyboxVertList()
{
    Vertex_P west_quad[4] = {
        { MyVec4(-1000.0f,  -1000.0f, -1000.0f) },
        { MyVec4(-1000.0f,  -1000.0f,  1000.0f) },
        { MyVec4(-1000.0f,   1000.0f, -1000.0f) },
        { MyVec4(-1000.0f,   1000.0f,  1000.0f) },
    };

    Vertex_P north_quad[4] = {
        { MyVec4(-1000.0f, -1000.0f, 1000.0f) },
        { MyVec4( 1000.0f, -1000.0f, 1000.0f) },
        { MyVec4(-1000.0f,  1000.0f, 1000.0f) },
        { MyVec4( 1000.0f,  1000.0f, 1000.0f) },
    };

    Vertex_P east_quad[4] = {
        { MyVec4(1000.0f, -1000.0f,  1000.0f) },
        { MyVec4(1000.0f, -1000.0f, -1000.0f) },
        { MyVec4(1000.0f,  1000.0f,  1000.0f) },
        { MyVec4(1000.0f,  1000.0f, -1000.0f) },
    };

    Vertex_P south_quad[4] = {
        { MyVec4( 1000.0f, -1000.0f, -1000.0f) },
        { MyVec4(-1000.0f, -1000.0f, -1000.0f) },
        { MyVec4( 1000.0f,  1000.0f, -1000.0f) },
        { MyVec4(-1000.0f,  1000.0f, -1000.0f) },
    };

    Vertex_P top_quad[4] = {
        { MyVec4(-1000.0f, 1000.0f,  1000.0f) },
        { MyVec4( 1000.0f, 1000.0f,  1000.0f) },
        { MyVec4(-1000.0f, 1000.0f, -1000.0f) },
        { MyVec4( 1000.0f, 1000.0f, -1000.0f) },
    };

    Vertex_P bottom_quad[4] = {
        { MyVec4(-1000.0f, -1000.0f, -1000.0f) },
        { MyVec4( 1000.0f, -1000.0f, -1000.0f) },
        { MyVec4(-1000.0f, -1000.0f,  1000.0f) },
        { MyVec4( 1000.0f, -1000.0f,  1000.0f) },
    };

    m_skybox_vert_list.reset();
    m_skybox_vert_list.addQuad(west_quad);
    m_skybox_vert_list.addQuad(north_quad);
    m_skybox_vert_list.addQuad(east_quad);
    m_skybox_vert_list.addQuad(south_quad);
    m_skybox_vert_list.addQuad(top_quad);
    m_skybox_vert_list.addQuad(bottom_quad);
    m_skybox_vert_list.realize();
}


void Renderer::loadTextures()
{
    const ConfigRender &conf_render = GetConfig().render;
    
    // Load our textures.
    m_grass_tex   = std::make_unique<DrawTexture>(conf_render.landscape.grass_texture);
    m_dirt_tex    = std::make_unique<DrawTexture>(conf_render.landscape.dirt_texture);
    m_stone_tex   = std::make_unique<DrawTexture>(conf_render.landscape.stone_texture);
    m_bedrock_tex = std::make_unique<DrawTexture>(conf_render.landscape.bedrock_texture);

    m_hit_test_tex = std::make_unique<DrawTexture>("hit_test.png");

    m_skybox_tex = std::make_unique<DrawCubemapTexture>(
        conf_render.sky.north_texture,
        conf_render.sky.south_texture,
        conf_render.sky.east_texture,
        conf_render.sky.west_texture,
        conf_render.sky.top_texture,
        conf_render.sky.bottom_texture);
}


bool Renderer::loadShaders()
{
    const ConfigRender &conf_render = GetConfig().render;

    bool success;

    // Init our draw state for the sky.
    DrawStateSettings skybox_settings;
    skybox_settings.title = "skybox";
    skybox_settings.enable_blending   = false;
    skybox_settings.enable_depth_test = false;
    skybox_settings.depth_func        = GL_ALWAYS;
    skybox_settings.draw_mode         = GL_TRIANGLES;
    skybox_settings.vert_shader_fname = conf_render.sky.vert_shader;
    skybox_settings.frag_shader_fname = conf_render.sky.frag_shader;

    m_skybox_DS = std::make_unique<DrawState_P>(1);

    success = (
        m_skybox_DS->addUniformMatrix4by4("mat_frustum") &&
        m_skybox_DS->addUniformMatrix4by4("mat_frustum_rotate") &&
        m_skybox_DS->create(skybox_settings));

    if (!success) {
        PrintDebug("Could not create the sky cube draw state. Bye!\n");
        return false;
    }

    // Init our draw state for landscapes.
    DrawStateSettings landscape_settings;
    landscape_settings.title = "landscape";
    landscape_settings.enable_blending   = true;
    landscape_settings.enable_depth_test = true;
    landscape_settings.depth_func        = GL_LEQUAL;
    landscape_settings.draw_mode         = GL_TRIANGLE_STRIP;
    landscape_settings.vert_shader_fname = conf_render.landscape.vert_shader;
    landscape_settings.frag_shader_fname = conf_render.landscape.frag_shader;

    m_landscape_DS = std::make_unique<DrawState_PNT>(1);

    success = (
        m_landscape_DS->addUniformMatrix4by4("mat_frustum") &&
        m_landscape_DS->addUniformFloat("fade_distance") &&
        m_landscape_DS->addUniformFloat("draw_distance") &&
        m_landscape_DS->addUniformFloat("camera_yaw")    &&
        m_landscape_DS->addUniformFloat("camera_pitch")  &&
        m_landscape_DS->addUniformVec4("camera_pos")     &&
        m_landscape_DS->create(landscape_settings));

    if (!success) {
        PrintDebug("Could not create the landscape draw state. Bye!\n");
        return false;
    }

    // Init our draw state for hit tests.
    DrawStateSettings hit_test_settings;
    hit_test_settings.title = "hit_test";
    hit_test_settings.enable_blending   = true;
    hit_test_settings.enable_depth_test = true;
    hit_test_settings.depth_func        = GL_LEQUAL;
    hit_test_settings.draw_mode         = GL_TRIANGLE_STRIP;
    hit_test_settings.vert_shader_fname = conf_render.hit_test.vert_shader;
    hit_test_settings.frag_shader_fname = conf_render.hit_test.frag_shader;

    m_hit_test_DS = std::make_unique<DrawState_PT>(1);

    success = (
        m_hit_test_DS->addUniformMatrix4by4("mat_frustum") &&
        m_hit_test_DS->addUniformFloat("fade_distance") &&
        m_hit_test_DS->addUniformFloat("draw_distance") &&
        m_hit_test_DS->addUniformFloat("camera_yaw")    &&
        m_hit_test_DS->addUniformFloat("camera_pitch")  &&
        m_hit_test_DS->addUniformVec4("camera_pos")     &&
        m_hit_test_DS->create(hit_test_settings));

    if (!success) {
        PrintDebug("Could not create the draw state for hit tests. Bye!\n");
        return false;
    }
   
    return true;
}



// Get the list of all the chunks we want to render.
void Renderer::getChunksToRender(std::vector<const Chunk *> *pOut_list, RenderStats *pOut_stats)
{
    GLfloat field_of_view = GetConfig().render.field_of_view;
    int     eval_blocks   = GetConfig().logic.eval_blocks;
    GLfloat draw_distance = GetConfig().logic.getDrawDistanceCm();

    const MyVec4 &camera_pos = m_world.getCameraPos();

    GLfloat camera_pitch = m_world.getCameraPitch();
    GLfloat camera_yaw   = m_world.getCameraYaw();
    
    pOut_stats->chunks_considered = 0;
    pOut_stats->chunks_rendered   = 0;

    if (MAGIC_BREAKPOINT) {
        printf("");
    }

    // Build our left and right frustum planes.
    MyMatrix4by4 pitch_rotate = MyMatrix4by4::RotateX(-camera_pitch);

    GLfloat offset = field_of_view / 2.0f;

    MyMatrix4by4 left_rotate = MyMatrix4by4::RotateY(camera_yaw - offset);
    MyVec4  left_dir_1 = left_rotate.times(VEC4_EASTWARD);
    MyVec4  left_dir_2 = pitch_rotate.times(left_dir_1);
    MyPlane left_clip_plane = MyRay(camera_pos, left_dir_2).toPlane();

    MyMatrix4by4 right_rotate = MyMatrix4by4::RotateY(camera_yaw + offset);
    MyVec4  right_dir_1 = right_rotate.times(VEC4_WESTWARD);
    MyVec4  right_dir_2 = pitch_rotate.times(right_dir_1);
    MyPlane right_clip_plane = MyRay(camera_pos, right_dir_2).toPlane();

    // Look up every chunk within our eval region.
    EvalRegion region = WorldPosToEvalRegion(camera_pos, eval_blocks);

    for     (int x = region.west();  x <= region.east();  x += CHUNK_WIDTH) {
        for (int z = region.south(); z <= region.north(); z += CHUNK_WIDTH) {
            pOut_stats->chunks_considered++;

            ChunkOrigin origin(x, z);
            const Chunk *chunk = m_world.getRequiredChunk(origin);

            // These must have been realized already.
            assert(chunk->isLandcsapeRealized());

            // Only keep the chunks within the view frustum.
            bool keep = chunk->isAbovePlane(left_clip_plane) &&
                        chunk->isAbovePlane(right_clip_plane);
            if (keep) {
                pOut_list->emplace_back(chunk);
                pOut_stats->chunks_rendered++;
            }
        }
    }
}


// Render the world.
RenderStats Renderer::renderWorld()
{
    const ConfigRender &conf_render = GetConfig().render;

    RenderStats stats;

    // Rebuild our uniform matrices.
    rebuildUniformMatrices();

    // Render the sky.
    renderSkybox(&stats);

    // Build a list of all our chunks.
    std::vector<const Chunk *> chunk_list;
    getChunksToRender(&chunk_list, &stats);

    // Render our surfaces.
    renderLandscapeList(SURF_GRASS,   chunk_list, *m_grass_tex,   &stats);
    renderLandscapeList(SURF_DIRT,    chunk_list, *m_dirt_tex,    &stats);
    renderLandscapeList(SURF_STONE,   chunk_list, *m_stone_tex,   &stats);

    // Finally, render our hit test.
    renderHitTest(&stats);

    return stats;
}



// Render our sky.
void Renderer::renderSkybox(RenderStats *pOut_stats)
{
    // Pluck out what we need from the game world.
    GLfloat camera_yaw   = m_world.getCameraYaw();
    GLfloat camera_pitch = m_world.getCameraPitch();

    m_skybox_DS->updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);
    m_skybox_DS->updateUniformMatrix4by4("mat_frustum_rotate", m_frustum_rotate_matrix);

    m_skybox_DS->updateUniformCubemapTexture(0, *m_skybox_tex);

    m_skybox_DS->render(m_skybox_vert_list);

    pOut_stats->state_changes++;
}



// Render one of our landscapes. This should use standard depth testing, and no blending.
void Renderer::renderLandscapeList(
    SurfaceType surf, const std::vector<const Chunk *> &chunk_list, 
    const DrawTexture &tex, RenderStats *pOut_stats)
{
    bool worth_doing = false;
    for (const Chunk *pChunk : chunk_list) {
        if (pChunk->getCountForSurface(surf) > 0) {
            worth_doing = true;
            break;
        }
    }

    if (!worth_doing) {
        return;
    }

    // Pluck out what we need from the game world.
    GLfloat fade_distance_cm = GetConfig().render.getFadeDistanceCm();
    GLfloat draw_distance_cm = GetConfig().logic.getDrawDistanceCm();

    GLfloat camera_yaw   = m_world.getCameraYaw();
    GLfloat camera_pitch = m_world.getCameraPitch();
    MyVec4  camera_pos   = m_world.getCameraPos();

    m_landscape_DS->updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);   
    m_landscape_DS->updateUniformFloat("fade_distance", fade_distance_cm);
    m_landscape_DS->updateUniformFloat("draw_distance", draw_distance_cm);

    m_landscape_DS->updateUniformFloat("camera_yaw",   camera_yaw);
    m_landscape_DS->updateUniformFloat("camera_pitch", camera_pitch);
    m_landscape_DS->updateUniformVec4("camera_pos",    camera_pos);

    m_landscape_DS->updateUniformTexture(0, tex);

    for (const Chunk *pChunk : chunk_list) {
        const VertList_PNT *vert_list = pChunk->getSurfaceList_RO(surf);
        if (vert_list != nullptr) {
            int item_count = vert_list->getItemCount();
            if (item_count > 0) {
                m_landscape_DS->render(*vert_list);
                pOut_stats->triangle_count += vert_list->getTriangleCount();
            }
        }
    }

    pOut_stats->state_changes++;
}


// Render the hit-test program.
// Don't use depth testing here, since it overlays the rest. Allow blending.
void Renderer::renderHitTest(RenderStats *pOut_stats)
{
    GLfloat fade_distance_cm = GetConfig().render.getFadeDistanceCm();
    GLfloat draw_distance_cm = GetConfig().logic.getDrawDistanceCm();

    GLfloat camera_yaw   = m_world.getCameraYaw();
    GLfloat camera_pitch = m_world.getCameraPitch();
    MyVec4  camera_pos   = m_world.getCameraPos();

    const VertList_PT &vert_list = m_world.getHitTestVertList();
    int item_count = vert_list.getItemCount();
    if (item_count > 0) {
        m_hit_test_DS->updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);
        m_hit_test_DS->updateUniformFloat("fade_distance", fade_distance_cm);
        m_hit_test_DS->updateUniformFloat("draw_distance", draw_distance_cm);

        m_hit_test_DS->updateUniformFloat("camera_yaw",   camera_yaw);
        m_hit_test_DS->updateUniformFloat("camera_pitch", camera_pitch);
        m_hit_test_DS->updateUniformVec4("camera_pos",    camera_pos);

        m_hit_test_DS->updateUniformTexture(0, *m_hit_test_tex);

        m_hit_test_DS->render(vert_list);

        pOut_stats->state_changes++;
        pOut_stats->triangle_count += vert_list.getTriangleCount();
    }
}

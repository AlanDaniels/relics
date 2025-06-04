
#include "stdafx.h"
#include "common_util.h"
#include "renderer.h"

#include "config.h"
#include "draw_state_p.h"
#include "draw_state_pnt.h"
#include "draw_state_pt.h"
#include "draw_texture.h"
#include "player.h"
#include "resource_pool.h"


bool Renderer::init()
{
    buildSkyboxVertList();
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
    GLfloat camera_pitch = m_world.getPlayer().getCameraPitch();
    GLfloat camera_yaw   = m_world.getPlayer().getCameraYaw();

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
    m_skybox_vert_list.update();
}


// Get the list of all the chunks we want to render.
std::vector<const Chunk *> Renderer::getChunksToRender(RenderStats *pOut_stats)
{
    std::vector<const Chunk *> results;

    GLfloat field_of_view    = GetConfig().render.field_of_view;
    int     eval_block_count = GetConfig().logic.eval_block_count;
    GLfloat draw_distance    = GetConfig().logic.getDrawDistanceCm();

    const Player &player = m_world.getPlayer();
    MyVec4  camera_pos   = player.getCameraPos();
    GLfloat camera_pitch = player.getCameraPitch();
    GLfloat camea_yaw    = player.getCameraYaw();
    
    pOut_stats->chunks_considered = 0;
    pOut_stats->chunks_rendered   = 0;

    // Build our left and right frustum planes.
    MyMatrix4by4 pitch_rotate = MyMatrix4by4::RotateX(-camera_pitch);

    GLfloat offset = field_of_view / 2.0f;

    MyMatrix4by4 left_rotate = MyMatrix4by4::RotateY(camea_yaw - offset);
    MyVec4  left_dir_1 = left_rotate.times(VEC4_EASTWARD);
    MyVec4  left_dir_2 = pitch_rotate.times(left_dir_1);
    MyPlane left_clip_plane = MyRay(camera_pos, left_dir_2).toPlane();

    MyMatrix4by4 right_rotate = MyMatrix4by4::RotateY(camea_yaw + offset);
    MyVec4  right_dir_1 = right_rotate.times(VEC4_WESTWARD);
    MyVec4  right_dir_2 = pitch_rotate.times(right_dir_1);
    MyPlane right_clip_plane = MyRay(camera_pos, right_dir_2).toPlane();

    // Look up every chunk within our eval region.
    EvalRegion region = WorldPosToEvalRegion(camera_pos, eval_block_count);

    for     (int x = region.west();  x <= region.east();  x += CHUNK_WIDTH) {
        for (int z = region.south(); z <= region.north(); z += CHUNK_WIDTH) {
            pOut_stats->chunks_considered++;

            ChunkOrigin origin(x, z);
            const Chunk *chunk = m_world.getChunk(origin);

            // Only keep the chunks within the view frustum.
            if (chunk != nullptr) {
                bool keep = chunk->isAbovePlane(left_clip_plane) &&
                    chunk->isAbovePlane(right_clip_plane);
                if (keep) {
                    results.emplace_back(chunk);
                    pOut_stats->chunks_rendered++;
                }
            }
        }
    }

    return std::move(results);
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
    std::vector<const Chunk *> chunk_vec = getChunksToRender(&stats);

    // Render our surfaces.
    const auto &pool = GetResourcePool();
    const auto &grass_tex = pool.getGrassTexture();
    const auto &dirt_tex  = pool.getDirtTexture();
    const auto &stone_tex = pool.getStoneTexture();
    const auto &coal_tex  = pool.getCoalTexture();

    renderLandscapeList(SurfaceType::GRASS_TOP, chunk_vec, grass_tex, &stats);
    renderLandscapeList(SurfaceType::DIRT,      chunk_vec, dirt_tex,  &stats);
    renderLandscapeList(SurfaceType::STONE,     chunk_vec, stone_tex, &stats);
    renderLandscapeList(SurfaceType::COAL,      chunk_vec, coal_tex,  &stats);

    // Render any wavefront objects.
    renderWFObjects(chunk_vec, &stats);

    // Finally, render our hit test.
    renderHitTest(&stats);

    return stats;
}



// Render our sky.
void Renderer::renderSkybox(RenderStats *pOut_stats)
{
    const auto &pool = GetResourcePool();
    const auto &skybox_tex = pool.getSkyboxTexture();
    const auto &skybox_ds  = pool.getSkyboxDrawState();

    skybox_ds.updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);
    skybox_ds.updateUniformMatrix4by4("mat_frustum_rotate", m_frustum_rotate_matrix);
    skybox_ds.updateUniformCubemapTexture(0, skybox_tex);
    skybox_ds.render(m_skybox_vert_list);

    pOut_stats->state_changes++;
}


// Render one of our landscapes. This should use standard depth testing, and no blending.
void Renderer::renderLandscapeList(
    SurfaceType surf, const std::vector<const Chunk *> &chunk_vec, 
    const DrawTexture &tex, RenderStats *pOut_stats)
{
    bool worth_doing = false;
    for (auto &iter : chunk_vec) {
        if (iter->landscape.getCountForSurface(surf) > 0) {
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

    const Player &player = m_world.getPlayer();
    GLfloat camera_yaw   = player.getCameraYaw();
    GLfloat camera_pitch = player.getCameraPitch();
    MyVec4  camera_pos   = player.getCameraPos();

    const auto &landscape_ds = GetResourcePool().getLandscapeDrawState();

    landscape_ds.updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);   
    landscape_ds.updateUniformFloat("fade_distance", fade_distance_cm);
    landscape_ds.updateUniformFloat("draw_distance", draw_distance_cm);

    landscape_ds.updateUniformFloat("camera_yaw",   camera_yaw);
    landscape_ds.updateUniformFloat("camera_pitch", camera_pitch);
    landscape_ds.updateUniformVec4("camera_pos",    camera_pos);

    landscape_ds.updateUniformTexture(0, tex);

    for (auto iter : chunk_vec) {
        const VertList_PNT *vert_list = iter->landscape.getSurfaceList_RO(surf);
        if (vert_list != nullptr) {
            int item_count = vert_list->getItemCount();
            if (item_count > 0) {
                landscape_ds.render(*vert_list);
                pOut_stats->triangle_count += vert_list->getTriCount();
            }
        }
    }

    pOut_stats->state_changes++;
}


// Render our wavefront objects.
// TODO: For now, just get this working. Worry about speed later.
void Renderer::renderWFObjects(
    std::vector<const Chunk *> &chunk_list, RenderStats *pOut_stats)
{
    GLfloat fade_distance_cm = GetConfig().render.getFadeDistanceCm();
    GLfloat draw_distance_cm = GetConfig().logic.getDrawDistanceCm();

    const Player &player = m_world.getPlayer();
    GLfloat camera_yaw   = player.getCameraYaw();
    GLfloat camera_pitch = player.getCameraPitch();
    MyVec4  camera_pos   = player.getCameraPos();

    const auto &wavefront_ds = GetResourcePool().getWavefrontDrawState();

    wavefront_ds.updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);
    wavefront_ds.updateUniformFloat("fade_distance", fade_distance_cm);
    wavefront_ds.updateUniformFloat("draw_distance", draw_distance_cm);

    wavefront_ds.updateUniformFloat("camera_yaw",   camera_yaw);
    wavefront_ds.updateUniformFloat("camera_pitch", camera_pitch);
    wavefront_ds.updateUniformVec4 ("camera_pos",   camera_pos);

    for (const auto &chunk_it : chunk_list) {
        for (const auto &instance : chunk_it->getWFInstances()) {
            const WFObject &original = instance->getOriginal();

            for (const std::string &group_name : original.getGroupNames()) {
                const auto &face_group   = original.getGroup(group_name);
                const auto *draw_texture = face_group.getMaterial()->getDrawTexture();
                const auto &vert_list    = face_group.getVertList();

                wavefront_ds.updateUniformTexture(0, *draw_texture);
                wavefront_ds.render(vert_list);
            }
        }
    }
}


// Render the hit-test program.
// Don't use depth testing here, since it overlays the rest. Allow blending.
void Renderer::renderHitTest(RenderStats *pOut_stats)
{
    const auto &pool = GetResourcePool();
    const auto &hit_test_tex = pool.getHitTestTexture();
    const auto &hit_test_ds  = pool.getHitTestDrawState();

    GLfloat fade_distance_cm = GetConfig().render.getFadeDistanceCm();
    GLfloat draw_distance_cm = GetConfig().logic.getDrawDistanceCm();

    const Player &player = m_world.getPlayer();
    MyVec4  camera_pos   = player.getCameraPos();
    GLfloat camera_yaw   = player.getCameraYaw();
    GLfloat camera_pitch = player.getCameraPitch();

    const VertList_PT &vert_list = m_world.getHitTestVertList();
    int item_count = vert_list.getItemCount();
    if (item_count > 0) {
        hit_test_ds.updateUniformMatrix4by4("mat_frustum", m_frustum_matrix);
        hit_test_ds.updateUniformFloat("fade_distance", fade_distance_cm);
        hit_test_ds.updateUniformFloat("draw_distance", draw_distance_cm);
        hit_test_ds.updateUniformFloat("camera_yaw",   camera_yaw);
        hit_test_ds.updateUniformFloat("camera_pitch", camera_pitch);
        hit_test_ds.updateUniformVec4("camera_pos",    camera_pos);
        hit_test_ds.updateUniformTexture(0, hit_test_tex);
        hit_test_ds.render(vert_list);

        pOut_stats->state_changes++;
        pOut_stats->triangle_count += vert_list.getTriCount();
    }
}

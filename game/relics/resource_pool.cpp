
#include "stdafx.h"
#include "resource_pool.h"

#include "config.h"
#include "draw_cubemap_texture.h"
#include "draw_state_p.h"
#include "draw_state_pt.h"
#include "draw_state_pnt.h"
#include "draw_texture.h"
#include "common_util.h"


// Our one expedient resource pool.
static ResourcePool g_resource_pool;

bool LoadResourcePool() {
    return g_resource_pool.load();
}

const ResourcePool &GetResourcePool() {
    return g_resource_pool;
}


// Reload all of our resoureces.
// The main "Config" object needs to be loaded first.
// Return bool if we couldn't find all of our resources.
bool ResourcePool::load()
{
    if (!loadTextures()) {
        return false;
    }

    if (!loadShaders()) {
        return false;
    }

    return true;
}


// Reload all of our textures.
bool ResourcePool::loadTextures()
{
    // Clear out any old textures.
    m_grass_tex    = nullptr;
    m_dirt_tex     = nullptr;
    m_stone_tex    = nullptr;
    m_coal_tex     = nullptr;
    m_bedrock_tex  = nullptr;
    m_hit_test_tex = nullptr;
    m_skybox_tex   = nullptr;

    // Load our textures.
    const ConfigRender &conf_render = GetConfig().render;

    m_grass_tex    = DrawTexture::Create(conf_render.landscape.grass_texture);
    m_dirt_tex     = DrawTexture::Create(conf_render.landscape.dirt_texture);
    m_stone_tex    = DrawTexture::Create(conf_render.landscape.stone_texture);
    m_coal_tex     = DrawTexture::Create(conf_render.landscape.coal_texture);
    m_bedrock_tex  = DrawTexture::Create(conf_render.landscape.bedrock_texture);
    m_hit_test_tex = DrawTexture::Create(conf_render.hit_test.texture);

    m_skybox_tex = DrawCubemapTexture::Create(
        conf_render.skybox.north_texture,
        conf_render.skybox.south_texture,
        conf_render.skybox.east_texture,
        conf_render.skybox.west_texture,
        conf_render.skybox.top_texture,
        conf_render.skybox.bottom_texture);

    // Return true if they all loaded correctly.
    bool result = (
        (m_grass_tex    != nullptr) &&
        (m_dirt_tex     != nullptr) &&
        (m_stone_tex    != nullptr) &&
        (m_coal_tex     != nullptr) &&
        (m_bedrock_tex  != nullptr) &&
        (m_hit_test_tex != nullptr) &&
        (m_skybox_tex   != nullptr));
    return result;
}


bool ResourcePool::loadShaders()
{
    // Clear out the old.
    m_wavefront_draw_state = nullptr;
    m_landscape_draw_state = nullptr;
    m_skybox_draw_state    = nullptr;
    m_hit_test_draw_state  = nullptr;

    const ConfigRender &conf_render = GetConfig().render;

    // Init our Wavefront draw state.
    {
        DrawStateSettings settings;
        settings.title = "wavefront";
        settings.enable_blending   = true;
        settings.enable_depth_test = true;
        settings.depth_func = GL_LEQUAL;
        settings.draw_mode  = GL_TRIANGLES;
        settings.vert_shader_fname = conf_render.landscape.vert_shader;
        settings.frag_shader_fname = conf_render.landscape.frag_shader;

        auto result = std::make_unique<DrawState_PNT>(1);

        bool success = (
            result->addUniformMatrix4by4("mat_frustum") &&
            result->addUniformFloat("fade_distance") &&
            result->addUniformFloat("draw_distance") &&
            result->addUniformFloat("camera_yaw") &&
            result->addUniformFloat("camera_pitch") &&
            result->addUniformVec4("camera_pos") &&
            result->create(settings));

        if (!success) {
            PrintDebug("Could not create the Wavefront draw state. Bye!\n");
            return false;
        }

        m_wavefront_draw_state = std::move(result);
    }

    // Init our landscape draw state.
    {
        DrawStateSettings settings;
        settings.title = "landscape";
        settings.enable_blending   = true;
        settings.enable_depth_test = true;
        settings.depth_func = GL_LEQUAL;
        settings.draw_mode  = GL_TRIANGLES;
        settings.vert_shader_fname = conf_render.landscape.vert_shader;
        settings.frag_shader_fname = conf_render.landscape.frag_shader;

        auto result = std::make_unique<DrawState_PNT>(1);

        bool success = (
            result->addUniformMatrix4by4("mat_frustum") &&
            result->addUniformFloat("fade_distance") &&
            result->addUniformFloat("draw_distance") &&
            result->addUniformFloat("camera_yaw") &&
            result->addUniformFloat("camera_pitch") &&
            result->addUniformVec4("camera_pos") &&
            result->create(settings));

        if (!success) {
            PrintDebug("Could not create the landscape draw state. Bye!\n");
            return false;
        }

        m_landscape_draw_state = std::move(result);
    }

    // Init our skybox draw state.
    {
        DrawStateSettings settings;
        settings.title = "skybox";
        settings.enable_blending   = false;
        settings.enable_depth_test = false;
        settings.depth_func = GL_ALWAYS;
        settings.draw_mode  = GL_TRIANGLES;
        settings.vert_shader_fname = conf_render.skybox.vert_shader;
        settings.frag_shader_fname = conf_render.skybox.frag_shader;

        auto result = std::make_unique<DrawState_P>(1);

        bool success = (
            result->addUniformMatrix4by4("mat_frustum") &&
            result->addUniformMatrix4by4("mat_frustum_rotate") &&
            result->create(settings));

        if (!success) {
            PrintDebug("Could not create the sky cube draw state. Bye!\n");
            return false;
        }

        m_skybox_draw_state = std::move(result);
    }

    // Init our hit test draw state.
    {
        DrawStateSettings settings;
        settings.title = "hit_test";
        settings.enable_blending   = true;
        settings.enable_depth_test = true;
        settings.depth_func = GL_LEQUAL;
        settings.draw_mode  = GL_TRIANGLES;
        settings.vert_shader_fname = conf_render.hit_test.vert_shader;
        settings.frag_shader_fname = conf_render.hit_test.frag_shader;

        auto result = std::make_unique<DrawState_PT>(1);

        bool success = (
            result->addUniformMatrix4by4("mat_frustum") &&
            result->addUniformFloat("fade_distance") &&
            result->addUniformFloat("draw_distance") &&
            result->addUniformFloat("camera_yaw") &&
            result->addUniformFloat("camera_pitch") &&
            result->addUniformVec4("camera_pos") &&
            result->create(settings));

        if (!success) {
            PrintDebug("Could not create the draw state for hit tests. Bye!\n");
            return false;
        }

        m_hit_test_draw_state = std::move(result);
    }
   
    // All done.
    return true;
}




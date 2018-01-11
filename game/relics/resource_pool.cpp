
#include "stdafx.h"
#include "resource_pool.h"

#include "config.h"
#include "draw_cubemap_texture.h"
#include "draw_texture.h"


// Our one expedient resource pool.
static ResourcePool resource_pool;

bool LoadResourcePool() {
    return resource_pool.load();
}

const ResourcePool &GetResourcePool() {
    return resource_pool;
}


// Reload all of our resoureces.
// The main "Config" object needs to be loaded first.
// Return bool if we couldn't find all of our resources.
bool ResourcePool::load()
{
    if (!loadTextures()) {
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
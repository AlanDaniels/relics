#pragma once

#include "stdafx.h"



class DrawTexture;
class DrawCubemapTexture;


// Pool all our resources in one place (textures, shaders,
// fonts), so that we can reload them at runtime, for debugging.
class ResourcePool
{
public:
    ResourcePool() {}
    ~ResourcePool() {}

    bool load();

    // A whole bunch of getters. Maybe we can optimize this later with enums.
    const DrawTexture &getGrassTexture()   const { return *m_grass_tex; }
    const DrawTexture &getDirtTexture()    const { return *m_dirt_tex; }
    const DrawTexture &getStoneTexture()   const { return *m_stone_tex; }
    const DrawTexture &getCoalTexture()    const { return *m_coal_tex; }
    const DrawTexture &getBedrockTexture() const { return *m_bedrock_tex; }
    const DrawTexture &getHitTestTexture() const { return *m_hit_test_tex; }

    const DrawCubemapTexture &getSkyboxTexture() const { return *m_skybox_tex; }

private:
    FORBID_COPYING(ResourcePool)
    FORBID_MOVING(ResourcePool)

    // Private methods.
    bool loadTextures();

    // Private data.
    std::unique_ptr<DrawTexture> m_grass_tex;
    std::unique_ptr<DrawTexture> m_dirt_tex;
    std::unique_ptr<DrawTexture> m_stone_tex;
    std::unique_ptr<DrawTexture> m_coal_tex;
    std::unique_ptr<DrawTexture> m_bedrock_tex;
    std::unique_ptr<DrawTexture> m_hit_test_tex;

    std::unique_ptr<DrawCubemapTexture> m_skybox_tex;
};


// Get at our one expedient global config object.
bool LoadResourcePool(); 

const ResourcePool &GetResourcePool();
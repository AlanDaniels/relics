#pragma once

#include "stdafx.h"

#include "draw_cubemap_texture.h"
#include "draw_state_p.h"
#include "draw_state_pt.h"
#include "draw_state_pnt.h"
#include "draw_texture.h"
#include "my_math.h"
#include "wavefront_object.h"


// Pool all our resources in one place (textures, shaders,
// fonts, etc), so that we can reload them at runtime, for debugging.
class ResourcePool
{
public:
    ResourcePool() {}
    ~ResourcePool() {}

    void clear();
    bool load();

    std::unique_ptr<WFInstance> cloneWFObject(const std::string &name, const MyVec4 &move) const;

    // A whole bunch of getters. Maybe we can optimize this later with enums.
    const DrawTexture &getGrassTexture()   const { return *m_grass_tex; }
    const DrawTexture &getDirtTexture()    const { return *m_dirt_tex; }
    const DrawTexture &getStoneTexture()   const { return *m_stone_tex; }
    const DrawTexture &getCoalTexture()    const { return *m_coal_tex; }
    const DrawTexture &getBedrockTexture() const { return *m_bedrock_tex; }
    const DrawTexture &getHitTestTexture() const { return *m_hit_test_tex; }

    const DrawCubemapTexture &getSkyboxTexture() const { return *m_skybox_tex; }

    const DrawState_PNT &getWavefrontDrawState() const { return *m_wavefront_draw_state; }
    const DrawState_PNT &getLandscapeDrawState() const { return *m_landscape_draw_state; }
    const DrawState_P   &getSkyboxDrawState()    const { return *m_skybox_draw_state; }
    const DrawState_PT  &getHitTestDrawState()   const { return *m_hit_test_draw_state; }

private:
    FORBID_COPYING(ResourcePool)
    FORBID_MOVING(ResourcePool)

    // Private methods.
    bool loadTextures();
    bool loadShaders();
    bool loadWFObjects();

    // Private data.
    std::unique_ptr<DrawTexture> m_grass_tex;
    std::unique_ptr<DrawTexture> m_dirt_tex;
    std::unique_ptr<DrawTexture> m_stone_tex;
    std::unique_ptr<DrawTexture> m_coal_tex;
    std::unique_ptr<DrawTexture> m_bedrock_tex;
    std::unique_ptr<DrawTexture> m_hit_test_tex;

    std::unique_ptr<DrawCubemapTexture> m_skybox_tex;

    std::unique_ptr<DrawState_PNT> m_wavefront_draw_state;
    std::unique_ptr<DrawState_PNT> m_landscape_draw_state;
    std::unique_ptr<DrawState_P>   m_skybox_draw_state;
    std::unique_ptr<DrawState_PT>  m_hit_test_draw_state;

    std::map<std::string, std::unique_ptr<WFObject>> m_wfobject_map;
};


// Get at our one expedient global resource pool.
void ClearResourcePool();
bool LoadResourcePool(); 
const ResourcePool &GetResourcePool();
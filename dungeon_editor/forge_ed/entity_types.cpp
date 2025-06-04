
#include "stdafx.h"
#include "entity_types.h"
#include "ed_resources.h"


EntityType::EntityType(
    const std::string &category, 
    const std::string &name,
    const std::string &icon_path) :
    m_category(category),
    m_name(name),
    m_icon_path(icon_path)
{
    // Try loading the icon.
    m_pIcon = LoadPNGBitmap(icon_path);
}


EntityType::~EntityType()
{
    if (m_pIcon != nullptr) {
        delete m_pIcon;
        m_pIcon = nullptr;
    }
}


// When we fire up the app, load all the entity types.
std::vector<EntityType *> g_entity_types;


// Load just one entity. If we couldn't open the icon for it, then never mind.
bool LoadOneEntity(
    const std::string &category, const std::string &name, const std::string &icon_path)
{
    EntityType *entity_type = new EntityType(category, name, icon_path);

    if (entity_type->getIcon() == nullptr) {
        delete entity_type;
        FreeEntityTypes();
        return false;
    }

    g_entity_types.emplace_back(entity_type);
    return true;
}


bool LoadEntityTypes() {
    assert(g_entity_types.size() == 0);

    return (
        LoadOneEntity("door", "player_start", "player_start.png") &&
        LoadOneEntity("door", "player_exit", "player_exit.png"));
}


bool FreeEntityTypes()
{
    for (auto iter : g_entity_types) {
        EntityType *pType = iter;
        delete pType;
    }
    g_entity_types.clear();
    return true;
}
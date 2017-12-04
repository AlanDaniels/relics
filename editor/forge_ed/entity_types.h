
#pragma once

#include "stdafx.h"


class EntityType
{
public:
    EntityType(
        const std::string &category, const std::string &name,
        const std::string &icon_path);
    ~EntityType();

    const std::string getCategory() const { return m_category; }
    const std::string getName() const { return m_name; }
    const wxBitmap *getIcon() const { return m_pIcon; }

private:
    // Forbid copying.
    EntityType(const EntityType &that) = delete;
    void operator=(const EntityType &that) = delete;

    std::string m_category;
    std::string m_name;
    std::string m_icon_path;

    wxBitmap *m_pIcon;
};


// When we fire up the app, load all the entity types.
extern std::vector<EntityType *> g_entity_types;

bool LoadEntityTypes();
bool FreeEntityTypes();
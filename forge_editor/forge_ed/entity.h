
#pragma once

#include "stdafx.h"


class GameEntity
{
    GameEntity(const std::string &category, const std::string &name);
    ~GameEntity() {}

    const std::string getCategory() const { return m_category; }
    const std::string getName() const { return m_name; }

private:
    // Forbid copying.
    GameEntity(const GameEntity &that) = delete;
    void operator=(const GameEntity &that) = delete;

    std::string m_category;
    std::string m_name;
};
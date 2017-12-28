#pragma once

#include "stdafx.h"


// The stats for our build. Just simple data, no resources.
class BuildStats
{
public:
    // Default ctor.
    BuildStats() :
        m_dirt(0),
        m_stone(0),
        m_air(0),
        m_coal(0) {}

    // Copy ctor.
    BuildStats(const BuildStats &that) :
        m_dirt(that.m_dirt),
        m_stone(that.m_stone),
        m_air(that.m_air),
        m_coal(that.m_coal) {}

    // Copy operator.
    BuildStats &operator=(const BuildStats &that) {
        m_dirt  = that.m_dirt;
        m_stone = that.m_stone;
        m_air   = that.m_air;
        m_coal  = that.m_coal;
        return *this;
    }

    // Move ctor.
    BuildStats(BuildStats &&that) :
        m_dirt(that.m_dirt),
        m_stone(that.m_stone),
        m_air(that.m_air),
        m_coal(that.m_coal) {}

    // Move operator.
    BuildStats &operator=(BuildStats &&that) {
        m_dirt  = that.m_dirt;
        m_stone = that.m_stone;
        m_air   = that.m_air;
        m_coal  = that.m_coal;
        return *this;
    }

    // Adders and removers.
    void addDirt(int val)  { m_dirt  += val; }
    void addStone(int val) { m_stone += val; }
    void addAir(int val)   { m_air   += val; }
    void addCoal(int val)  { m_coal  += val; }

    int getTotal() const {
        return m_dirt + m_stone + m_air + m_coal;
    }

    // Show this as a string.
    std::string toString() const {
        std::string result = "";

        if (m_dirt  > 0) { result += fmt::format("Dirt blocks: {}\n",  m_dirt); }
        if (m_stone > 0) { result += fmt::format("Stone blocks: {}\n", m_stone); }
        if (m_air   > 0) { result += fmt::format("Air blocks: {}\n",   m_air); }
        if (m_coal  > 0) { result += fmt::format("Coal blocks: {}\n",  m_coal); }

        if (result.empty()) {
            result = "No data written!";
        }
        else {
            // Get that trailing newline.
            result.erase(result.length() - 1);
        }

        return result;
    }

private:
    // Private data.
    int  m_dirt;
    int  m_stone;
    int  m_air;
    int  m_coal;
};
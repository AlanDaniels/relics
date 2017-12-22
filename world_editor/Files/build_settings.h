#pragma once

#include "stdafx.h"

// The build settings for our World.
// Just a simple structure. No resources.
class BuildSettings
{
public:
    // Default ctor. Pick some reasonable defaults.
    BuildSettings() :
        m_height_map_fname(""),
        m_stone_pct(50.0),
        m_coal_pct(1.0) {}

    // Copy ctor.
    BuildSettings(const BuildSettings &that) :
        m_height_map_fname(that.m_height_map_fname),
        m_stone_pct(that.m_stone_pct),
        m_coal_pct(that.m_coal_pct) {}

    // Copy operator.
    BuildSettings &operator=(const BuildSettings &that) {
        m_height_map_fname = that.m_height_map_fname;
        m_stone_pct = that.m_stone_pct;
        m_coal_pct  = that.m_coal_pct;
        return *this;
    }

    // Move ctor.
    BuildSettings(BuildSettings &&that) :
        m_height_map_fname(std::move(that.m_height_map_fname)),
        m_stone_pct(that.m_stone_pct),
        m_coal_pct(that.m_coal_pct) {}

    // Move operator.
    BuildSettings &operator=(BuildSettings &&that) {
        m_height_map_fname = std::move(that.m_height_map_fname);
        m_stone_pct = that.m_stone_pct;
        m_coal_pct  = that.m_coal_pct;
        return *this;
    }

    // Destructor. Boring.
    ~BuildSettings() 
        {}

    // Getters.
    const std::string &getHeightMapFilename() const { return m_height_map_fname; }
    double getStonePct() const { return m_stone_pct; }
    double getCoalPct() const { return m_coal_pct; }

    // Setters.
    void setHeightMapFilename(const std::string &fname) {
        m_height_map_fname = fname;
    }

    void setStonePct(double val) {
        m_stone_pct = clamp(val);
    }

    void setCoalPct(double val) {
        m_coal_pct = clamp(val);
    }

private:
    // Private methods.
    double clamp(double val) {
        if (val > 100.0) {
            return 100.0;
        }
        else if (val < 0.0) {
            return 0.0;
        }
        else {
            return val;
        }
    }

    // Private data.
    std::string m_height_map_fname;
    double m_stone_pct;
    double m_coal_pct;
};

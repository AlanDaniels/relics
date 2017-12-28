#pragma once

#include "stdafx.h"
#include "format.h"
#include <boost/filesystem.hpp>



// The build settings for our World. Just simple data, no resources.
// For now, don't bother with any range checking. Just see what works.
class BuildSettings
{
public:
    // Default ctor. Pick some reasonable defaults.
    BuildSettings() :
        m_height_map_fname(""),
        m_stone_percent(50.0),
        m_stone_subtracted(4),
        m_stone_displacement(8),
        m_stone_noise_scale(100),
        m_coal_density(1.0) {}

    // Copy ctor.
    BuildSettings(const BuildSettings &that) :
        m_height_map_fname(that.m_height_map_fname),
        m_stone_percent(that.m_stone_percent),
        m_stone_subtracted(that.m_stone_subtracted),
        m_stone_displacement(that.m_stone_displacement),
        m_stone_noise_scale(that.m_stone_noise_scale),
        m_coal_density(that.m_coal_density) {}

    // Copy operator.
    BuildSettings &operator=(const BuildSettings &that) {
        m_height_map_fname   = that.m_height_map_fname;
        m_stone_percent      = that.m_stone_percent;
        m_stone_subtracted   = that.m_stone_subtracted;
        m_stone_displacement = that.m_stone_displacement;
        m_stone_noise_scale  = that.m_stone_noise_scale;
        m_coal_density       = that.m_coal_density;
        return *this;
    }

    // Move ctor.
    BuildSettings(BuildSettings &&that) :
        m_height_map_fname(std::move(that.m_height_map_fname)),
        m_stone_percent(that.m_stone_percent),
        m_stone_subtracted(that.m_stone_subtracted),
        m_stone_displacement(that.m_stone_displacement),
        m_stone_noise_scale(that.m_stone_noise_scale),
        m_coal_density(that.m_coal_density) {}

    // Move operator.
    BuildSettings &operator=(BuildSettings &&that) {
        m_height_map_fname   = std::move(that.m_height_map_fname);
        m_stone_percent      = that.m_stone_percent;
        m_stone_subtracted   = that.m_stone_subtracted;
        m_stone_displacement = that.m_stone_displacement;
        m_stone_noise_scale  = that.m_stone_noise_scale;
        m_coal_density       = that.m_coal_density;
        return *this;
    }

    // Destructor.
    ~BuildSettings() 
        {}

    // Getters.
    const std::string &getHeightMapFilename() const { return m_height_map_fname; }

    double getStonePercent()      const { return m_stone_percent; }
    double getStoneSubtracted()   const { return m_stone_subtracted; }
    double getStoneDisplacement() const { return m_stone_displacement; }
    double getStoneNoiseScale()   const { return m_stone_noise_scale; }
    double getCoalDensity()       const { return m_coal_density; }

    // The height map name can start off blank, but
    // once assigned, it has to be for a valid PNG file.
    bool setHeightMapFilename(const std::string &fname) { 
        if (boost::filesystem::is_regular_file(fname)) {
            m_height_map_fname = fname;
            return true;
        }
        else {
            showErrorMsg(fmt::format("'{}' is not a valid file name.", fname));
            return false;
        }
    }

    // Other setters. Return true for a valid value.
    bool setStonePercent(double val)      { m_stone_percent = val; return true;  }
    bool setStoneSubtracted(double val)   { m_stone_subtracted = val; return true; }
    bool setStoneDisplacement(double val) { m_stone_displacement = val; return true; }
    bool setStoneNoiseScale(double val)   { m_stone_noise_scale = val; return true; }
    bool setCoalDensity(double val)       { m_coal_density = val; return true; }

private:
    // Private methods.
    void showErrorMsg(const std::string &msg) {
        wxMessageBox(msg, "Build Settings", wxOK | wxICON_EXCLAMATION, nullptr);
    }

    // Private data.
    std::string m_height_map_fname;

    double m_stone_percent;
    double m_stone_subtracted;
    double m_stone_displacement;
    double m_stone_noise_scale;

    double m_coal_density;
};

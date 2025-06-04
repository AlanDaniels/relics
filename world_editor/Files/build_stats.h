#pragma once

#include "stdafx.h"
#include "common_util.h"


// The stats for our build. Just simple data, no resources.
class BuildStats
{
public:
    // Default ctor.
    BuildStats() {}

    // Copy ctor.
    BuildStats(const BuildStats &that) :
        m_counts(that.m_counts) {}

    // Copy operator.
    BuildStats &operator=(const BuildStats &that) {
        m_counts = that.m_counts;
        return *this;
    }

    // Move ctor.
    BuildStats(BuildStats &&that) :
        m_counts(that.m_counts) {}

    // Move operator.
    BuildStats &operator=(BuildStats &&that) {
        m_counts = that.m_counts;
        return *this;
    }

    // Adders and removers.
    void add(BlockType bt)  { 
        m_counts[bt]++;
    }

    int getTotal() const {
        int result = 0;
        for (const auto &iter : m_counts) {
            result += iter.second;
        }
    }

    // Show this as a string.
    std::string toString() const {
        std::string result = "";

        int dirt  = getCount(BlockType::DIRT);
        int stone = getCount(BlockType::STONE);
        int air   = getCount(BlockType::AIR);
        int coal  = getCount(BlockType::COAL);

        int total_blocks = dirt + stone + air + coal;
        int db_writes = 0;

        if (dirt > 0) {
            double dirt_pct = (dirt / ((double) total_blocks)) * 100.0;
            result += fmt::format(
                "Dirt:  {0} blocks ({1}), {2:0.1f}%\n",
                dirt, ReadableNumber(dirt), dirt_pct);

            db_writes++;
        }

        if (stone > 0) { 
            double stone_pct = (stone / ((double)total_blocks)) * 100.0;
            result += fmt::format(
                "Stone: {0} blocks ({1}), {2:0.1f}%\n",
                stone, ReadableNumber(stone), stone_pct); 

            db_writes++;
        }

        if (air > 0) { 
            double air_pct = (air / ((double)total_blocks)) * 100.0;
            result += fmt::format(
                "Air:   {0} blocks ({1}, {2:0.1f}%\n", 
                air, ReadableNumber(air), air_pct);

            db_writes += air;
        }

        if (coal > 0) {
            double coal_pct = (coal / ((double)total_blocks)) * 100.0;
            result += fmt::format(
                "Coal:  {0} blocks ({1}), {2:0.1f}%\n",
                coal, ReadableNumber(coal), coal_pct);

            db_writes += coal;
        }

        // How bad would this hammer the database?
        if (db_writes > 0) {
            result += fmt::format(
                "\nThis would result in {0} blocks written to the database ({1}).\n",
                db_writes, ReadableNumber(db_writes));
        }

        // All done. Get rid of that trailing newline.
        if (result.empty()) {
            result = "No data written!";
        }
        else {
            result.erase(result.length() - 1);
        }

        return std::move(result);
    }

private:
    // Private methods.
    int getCount(BlockType bt) const {
        const auto iter = m_counts.find(bt);
        if (iter == m_counts.end()) {
            return 0;
        }
        else {
            return iter->second;
        }
    }

    // Private data.
    std::map<BlockType, int> m_counts;
};
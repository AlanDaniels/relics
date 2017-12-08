
#pragma once

#include "stdafx.h"

/**
 * Note that our logic for our editor is a bit reversed from the game itself.
 * In the game, we think of X-Z being the "landscape", and Y being "up".
 * That's consistent with how OpenGL views the world, from the front.
 *
 * But, for our editor, it's a very top-down view, so we're going to keep
 * our sanity and think of X-Y as the lanscape" and "Z" being the top-down view.
 * That's consistent with how an image editor sees thw world.
 *
 * A bit inconsistent, but much easier to understand, and we'll just be
 * mindful when we're writing out our data to the SQLite database.
 */


class GameLevel;


// A point on our game map.
class XYCoord
{
public:
    XYCoord(int x, int y) :
        m_x(x), m_y(y) {}
    XYCoord(const XYCoord &that) :
        m_x(that.m_x), m_y(that.m_y) {}

    XYCoord &operator=(const XYCoord &that);
    bool operator<(const XYCoord &that) const;

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }

private: 
    // Disallow the default ctor.
    XYCoord() = delete;

    int m_x;
    int m_y;
};


// Possible block types.
// We only include "none" for the sake of erasing.
enum BlockType {
    BLOCK_TYPE_NONE,
    BLOCK_TYPE_FLOOR,
    BLOCK_TYPE_WALL,
};


// One tile block. For now, it just holds the tile type.
class Block 
{
public:
    Block(BlockType block_type) :
        m_block_type(block_type) {}

    Block(const Block &that) :
        m_block_type(that.m_block_type) {}

    Block &operator=(const Block &that) {
        m_block_type = that.m_block_type;
        return *this;
    }

    BlockType getBlockType() const { return m_block_type; }

private:
    // Forbid the default ctor.
    Block() = delete;

    BlockType m_block_type;
};


// A "change set", or, how we're going to have undos.
class ChangeSet
{
public:
    ChangeSet(const GameLevel &level) :
        m_game_level(level) {}
    ~ChangeSet() { clear(); }

    void clear();
    
    BlockType get(const XYCoord &pt) const;
    BlockType getUnderneath(const XYCoord &pt) const;
    void set(const XYCoord &pt, BlockType content);

    // Convenience methods.
    void drawBox (const XYCoord &pt1, const XYCoord &pt2, BlockType block_type);
    void drawLine(const XYCoord &pt1, const XYCoord &pt2, BlockType block_type);
    void drawRoom(const XYCoord &pt1, const XYCoord &pt2);

    void addOutsideWall(const XYCoord &pt);

    // An expedient hack.
    auto begin() const { return m_data.begin(); }
    auto end()   const { return m_data.end(); }
    auto find (const XYCoord &pt) const { return m_data.find(pt); }

private:
    // Disallow copying, and default ctor.
    ChangeSet() = delete;
    ChangeSet(const ChangeSet &that) = delete;
    void operator=(const ChangeSet &that) = delete;

    const GameLevel &m_game_level;
    std::map<XYCoord, std::unique_ptr<Block>> m_data;
};


// Our game level. Maybe down the road we can unify the
// code with the game's code, but for now, keep it simple.
class GameLevel
{
public:
    GameLevel() {}
    ~GameLevel();

    void applyChanges(const ChangeSet &change_set);

    BlockType get(const XYCoord &pt) const;
    void set(const XYCoord &pt, BlockType content);

    // An expedient hack.
    auto begin() const { return m_data.begin(); }
    auto end()   const { return m_data.end(); }
    auto find(const XYCoord &pt) const { return m_data.find(pt); }

private:
    // Disallow copying.
    GameLevel(const GameLevel &that) = delete;
    void operator=(const GameLevel &that) = delete;

    std::map<XYCoord, std::unique_ptr<Block>> m_data;
};

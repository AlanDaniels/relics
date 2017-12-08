
#include "stdafx.h"
#include "game_world.h"


// Game Point copy operator.
XYCoord &XYCoord::operator=(const XYCoord &that)
{
    m_x = that.m_x;
    m_y = that.m_y;
    return *this;
}


// Less than operator. We need this since we're using this as a map key.
bool XYCoord::operator<(const XYCoord &that) const
{
    if      (m_x < that.m_x) { return true;  }
    else if (m_x > that.m_x) { return false; }
    else if (m_y < that.m_y) { return true;  }
    else if (m_y > that.m_y) { return false; }
    else { return false; }
}


// Game level dtor. Clean everything up.
GameLevel::~GameLevel()
{
    m_data.clear();
}



// Get a tile type.
BlockType GameLevel::get(const XYCoord &pt) const
{
    auto iter = m_data.find(pt);
    if (iter == m_data.end()) {
        return BLOCK_TYPE_NONE;
    }
    else {
        return iter->second->getBlockType();
    }
}


// Set a block value.
void GameLevel::set(const XYCoord &pt, BlockType block_type)
{
    // If setting "none", remove any info.
    if (block_type == BLOCK_TYPE_NONE) {
        m_data.erase(pt);
    }

    // Otherwise, meh, just set it.
    else {
        m_data[pt] = std::make_unique<Block>(block_type);
    }
}


// Apply changes to a game level.
// TODO: Here is where you'd make an "undo" stack. Pretty challenging though!
void GameLevel::applyChanges(const ChangeSet &change_set)
{
    for (auto &cs_iter = change_set.begin(); cs_iter != change_set.end(); cs_iter++) {
        XYCoord pt = cs_iter->first;
        BlockType block_type = cs_iter->second->getBlockType();
        set(pt, block_type);
    }
}


// Clear everything out.
void ChangeSet::clear()
{
    m_data.clear();
}


// Get a tile type.
BlockType ChangeSet::get(const XYCoord &pt) const
{
    auto iter = m_data.find(pt);
    if (iter == m_data.end()) {
        return BLOCK_TYPE_NONE;
    }
    else {
        return iter->second->getBlockType();
    }
}


// If that doesn't work, check the original canvas.
BlockType ChangeSet::getUnderneath(const XYCoord &pt) const
{
    BlockType block = get(pt);
    if (block == BLOCK_TYPE_NONE) {
        block = m_game_level.get(pt);
    }
    return block;
}


// Set a block value.
void ChangeSet::set(const XYCoord &pt, BlockType block_type)
{
    // If setting "none", remove any info.
    if (block_type == BLOCK_TYPE_NONE) {
        m_data.erase(pt);
    }

    // Otherwise, meh, just set it.
    else {
        m_data[pt] = std::make_unique<Block>(block_type);
    }
}


// Draw a box.
void ChangeSet::drawBox(const XYCoord &pt1, const XYCoord &pt2, BlockType block_type)
{
    clear();

    int left, right;
    if (pt1.x() < pt2.x()) {
        left  = pt1.x();
        right = pt2.x();
    }
    else {
        left  = pt2.x();
        right = pt1.x();
    }

    int top, bottom;
    if (pt1.y() < pt2.y()) {
        bottom = pt1.y();
        top    = pt2.y();
    }
    else {
        top    = pt1.y();
        bottom = pt2.y();
    }

    for (int x = left; x <= right; x++) {
        for (int y = bottom; y <= top; y++) {
            set(XYCoord(x, y), block_type);
        }
    }
}


// Draw a line.
void ChangeSet::drawLine(const XYCoord &pt1, const XYCoord &pt2, BlockType block_type)
{
    clear();

    int left, right;
    if (pt1.x() < pt2.x()) {
        left  = pt1.x();
        right = pt2.x();
    }
    else {
        left  = pt2.x();
        right = pt1.x();
    }

    int top, bottom;
    if (pt1.y() < pt2.y()) {
        bottom = pt1.y();
        top    = pt2.y();
    }
    else {
        top    = pt1.y();
        bottom = pt2.y();
    }

    int diff_x = abs(left - right);
    int diff_y = abs(bottom - top);

    if (diff_x > diff_y) {
        for (int x = left; x <= right; x++) {
            set(XYCoord(x, pt1.y()), block_type);
        }
    }
    else {
        for (int y = bottom; y <= top; y++) {
            set(XYCoord(pt1.x(), y), block_type);
        }
    }
}


// Draw a room.
void ChangeSet::drawRoom(const XYCoord &pt1, const XYCoord &pt2)
{
    clear();

    int left, right;
    if (pt1.x() < pt2.x()) {
        left  = pt1.x();
        right = pt2.x();
    }
    else {
        left  = pt2.x();
        right = pt1.x();
    }

    int top, bottom;
    if (pt1.y() < pt2.y()) {
        bottom = pt1.y();
        top    = pt2.y();
    }
    else {
        top    = pt1.y();
        bottom = pt2.y();
    }

    // First, lay down floor.
    for (int x = left + 1; x <= right - 1; x++) {
        for (int y = bottom + 1; y <= top - 1; y++) {
            set(XYCoord(x, y), BLOCK_TYPE_FLOOR);
        }
    }

    // Then, march along the edges.
    // Any floor tile with an empty neighbor gets a wall.
    for (int x = left; x <= right; x++) {
        addOutsideWall(XYCoord(x, top));
        addOutsideWall(XYCoord(x, bottom));
    }

    for (int y = bottom; y <= top; y++) {
        addOutsideWall(XYCoord(left, y));
        addOutsideWall(XYCoord(right, y));
    }
}


// If a tile is empty, and has any floor neighbors, plant a wall there.
void ChangeSet::addOutsideWall(const XYCoord &pt)
{
    int W = pt.x() - 1;
    int x = pt.x();
    int E = pt.x() + 1;

    int N = pt.y() + 1;
    int y = pt.y();
    int S = pt.y() - 1;

    bool nothing_underneath = 
        (getUnderneath(pt) == BLOCK_TYPE_NONE);

    bool adjacent_to_floor = (
        (getUnderneath(XYCoord(W, N)) == BLOCK_TYPE_FLOOR) ||  // Northwest
        (getUnderneath(XYCoord(x, N)) == BLOCK_TYPE_FLOOR) ||  // North
        (getUnderneath(XYCoord(E, N)) == BLOCK_TYPE_FLOOR) ||  // Northeast
        (getUnderneath(XYCoord(W, y)) == BLOCK_TYPE_FLOOR) ||  // West
        (getUnderneath(XYCoord(E, y)) == BLOCK_TYPE_FLOOR) ||  // East
        (getUnderneath(XYCoord(W, S)) == BLOCK_TYPE_FLOOR) ||  // Southwest
        (getUnderneath(XYCoord(x, S)) == BLOCK_TYPE_FLOOR) ||  // South
        (getUnderneath(XYCoord(E, S)) == BLOCK_TYPE_FLOOR));   // Southeast

    if (nothing_underneath && adjacent_to_floor) {
        set(pt, BLOCK_TYPE_WALL);
    }
}

#pragma once

#include "stdafx.h"

#include "event_handler.h"
#include "my_math.h"

class GameWorld;


class Player
{
public:
    Player(GameWorld &world);

    void onGameTick(int elapsed_msec, const EventStateMsg &msg);

    void setOnSolidGround(bool val);
    void setPlayerPos(const MyVec4 &pos);
    void setCameraYaw(GLfloat val)   { m_camera_yaw = val;   clampRotations(); }
    void setCameraPitch(GLfloat val) { m_camera_pitch = val; clampRotations(); }

    // Getters.
    const GameWorld &getGameWorld() const { return m_game_world; }
    const MyVec4 &getPlayerPos() const { return m_player_pos; }

    MyBoundingBox getBoundingBox() const;
    MyRay getCameraRay() const;

    GLfloat getCameraPitch()     const { return m_camera_pitch; }
    GLfloat getCameraYaw()       const { return m_camera_yaw; }
    const MyVec4 &getCameraPos() const { return m_camera_pos; }

private:
    FORBID_DEFAULT_CTOR(Player)
    FORBID_COPYING(Player)
    FORBID_MOVING(Player)

    // Private methods.
    void calcNoclipMotion(int elapsed_msec, const EventStateMsg &msg);
    void calcStandardMotion(int elapsed_msec, const EventStateMsg &msg);


    void clampRotations();

    // Private data.
    GameWorld &m_game_world;

    bool   m_on_solid_ground;
    MyVec4 m_gravity_vec;

    MyVec4  m_player_pos;
    GLfloat m_camera_pitch;
    GLfloat m_camera_yaw;
    MyVec4  m_camera_pos;
};
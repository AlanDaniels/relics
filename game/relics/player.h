#pragma once

#include "stdafx.h"

#include "event_handler.h"
#include "my_math.h"


class Player
{
public:
    Player();

    void onGameTick(int elapsed_msec, const EventStateMsg &msg);

    void setPos(const MyVec4 &pos)   { m_camera_pos = pos; }
    void setCameraYaw(GLfloat val)   { m_camera_yaw = val;   clampRotations(); }
    void setCameraPitch(GLfloat val) { m_camera_pitch = val; clampRotations(); }

    // Getters.
    MyRay getCameraRay() const;

    GLfloat getCameraPitch() const { return m_camera_pitch; }
    GLfloat getCameraYaw()   const { return m_camera_yaw; }
    const MyVec4 &getCameraPos() const { return m_camera_pos; }

private:
    FORBID_COPYING(Player)
    FORBID_MOVING(Player)

    // Private methods.
    void clampRotations();

    // Private data.
    GLfloat  m_camera_pitch;
    GLfloat  m_camera_yaw;
    MyVec4   m_camera_pos;
};
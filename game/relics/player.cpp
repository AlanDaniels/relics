
#include "stdafx.h"
#include "player.h"

#include "config.h"


Player::Player() :
    m_camera_pitch(0.0f),
    m_camera_yaw(0.0f),
    m_camera_pos(0, 0, 0)
{
}


// Get the camera's eye-ray. Eh, maybe we could cache this.
MyRay Player::getCameraRay() const
{
    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);
    MyMatrix4by4 rotator = roty.times(rotx);

    MyVec4 camera_normal = rotator.times(VEC4_NORTHWARD);
    return MyRay(m_camera_pos, camera_normal);
}


// Handle a game tick. This should always be called through the 
// "GameEventHandler" object, which will deal with keyboard events.
void Player::onGameTick(int elapsed_msec, const EventStateMsg &msg)
{
    GLfloat degrees_per_pixel = GetConfig().window.mouse_degrees_per_pixel;

    // Handle the mouse movement.
    int mouse_diff_x = msg.getMouseDiffX();
    int mouse_diff_y = msg.getMouseDiffY();
    m_camera_yaw   += static_cast<GLfloat>(mouse_diff_x) * degrees_per_pixel;
    m_camera_pitch -= static_cast<GLfloat>(mouse_diff_y) * degrees_per_pixel;
    clampRotations();

    // Then, handle the camera movement.
    // TODO: The math here is tricky, and should we tie it to the elapsed time?
    GLfloat meters_per_sec = GetConfig().debug.noclip_flight_speed;
    GLfloat centimeters = (meters_per_sec / 10.0f) * elapsed_msec;

    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);
    MyMatrix4by4 rotator = roty.times(rotx);

    GLfloat boost = msg.getSpeedBoost() ? 10.0f : 1.0f;

    if (msg.getMoveFwd()) {
        MyVec4 rotated = rotator.times(VEC4_NORTHWARD);
        MyVec4 move    = rotated.times(centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveBkwd()) {
        MyVec4 rotated = rotator.times(VEC4_NORTHWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    if (msg.getMoveLeft()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveRight()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    if (msg.getMoveUp()) {
        MyVec4 move = VEC4_UPWARD.times(centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }

    else if (msg.getMoveDown()) {
        MyVec4 move = VEC4_UPWARD.times(-centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_camera_pos = tr.times(m_camera_pos);
    }
}


// Clamp the player's eye rotations.
void Player::clampRotations()
{
    while (m_camera_yaw > 180.0) {
        m_camera_yaw -= 360.0;
    }
    while (m_camera_yaw <= -180.0) {
        m_camera_yaw += 360.0;
    }

    if (m_camera_pitch > 90.0) {
        m_camera_pitch = 90.0;
    }
    else if (m_camera_pitch < -90.0) {
        m_camera_pitch = -90.0;
    }
}

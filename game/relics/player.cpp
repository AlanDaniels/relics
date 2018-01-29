
#include "stdafx.h"
#include "player.h"

#include "config.h"


// The player's dimensions, in centimeters.
const GLfloat PLAYER_GRAVITY = 30.0f;
const int PLAYER_HEIGHT = 190;
const int PLAYER_WIDTH  =  90;
const int PLAYER_EYE_LEVEL = 160;


Player::Player(GameWorld &world) :
    m_game_world(world),
    m_apply_gravity(false),
    m_gravity_vec(0, 0, 0),
    m_camera_pitch(0.0f),
    m_camera_yaw(0.0f),
    m_camera_pos(0, 0, 0)
{
}


// Set the player's position.
void Player::setPlayerPos(const MyVec4 &pos) 
{
    m_player_pos = pos;
    
    MyVec4 eye_offset(0, PLAYER_EYE_LEVEL, 0);
    m_camera_pos = pos.plus(eye_offset);
}


// Get the player's bounding box.
MyBoundingBox Player::getBoundingBox() const
{
    GLfloat min_x = m_player_pos.x() - (PLAYER_WIDTH / 2);
    GLfloat max_x = m_player_pos.x() + (PLAYER_WIDTH / 2);
    GLfloat min_y = m_player_pos.y();
    GLfloat max_y = m_player_pos.y() + PLAYER_HEIGHT;
    GLfloat min_z = m_player_pos.z() - (PLAYER_WIDTH / 2);
    GLfloat max_z = m_player_pos.z() + (PLAYER_WIDTH / 2);

    MyVec4 lower(min_x, min_y, min_z);
    MyVec4 upper(max_x, max_y, max_z);
    return MyBoundingBox(lower, upper);
}


// Get the player's eye-position camera ray. Eh, maybe we could cache this.
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
        m_player_pos = tr.times(m_player_pos);
    }

    else if (msg.getMoveBkwd()) {
        MyVec4 rotated = rotator.times(VEC4_NORTHWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_player_pos = tr.times(m_player_pos);
    }

    if (msg.getMoveLeft()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(-centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_player_pos = tr.times(m_player_pos);
    }

    else if (msg.getMoveRight()) {
        MyVec4 rotated = rotator.times(VEC4_EASTWARD);
        MyVec4 move    = rotated.times(centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_player_pos = tr.times(m_player_pos);
    }

    if (msg.getMoveUp()) {
        MyVec4 move = VEC4_UPWARD.times(centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_player_pos = tr.times(m_player_pos);
    }

    else if (msg.getMoveDown()) {
        MyVec4 move = VEC4_UPWARD.times(-centimeters * boost);
        MyMatrix4by4 tr = MyMatrix4by4::Translate(move);
        m_player_pos = tr.times(m_player_pos);
    }

    // Apply gravity.
    if (m_apply_gravity) {
        GLfloat gravity_per_frame = 
            static_cast<GLfloat>((PLAYER_GRAVITY * BLOCK_SCALE) / (FRAME_DELTA_MSECS * 1000.0));
        m_gravity_vec = m_gravity_vec.plus(MyVec4(0, -gravity_per_frame, 0));

        MyMatrix4by4 tr = MyMatrix4by4::Translate(m_gravity_vec);
        m_player_pos = tr.times(m_player_pos);
    }

    // All done. Update the camera as well.
    MyVec4 eye_offset(0, PLAYER_EYE_LEVEL, 0);
    m_camera_pos = m_player_pos.plus(eye_offset);
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

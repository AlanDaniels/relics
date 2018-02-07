
#include "stdafx.h"
#include "player.h"

#include "config.h"
#include "format.h"
#include "heads_up_display.h"


// The player's dimensions, in centimeters.
const int PLAYER_HEIGHT = 180;
const int PLAYER_WIDTH  =  90;
const int PLAYER_EYE_LEVEL = 160;


Player::Player(GameWorld &world) :
    m_game_world(world),
    m_on_solid_ground(false),
    m_vert_motion(0, 0, 0),
    m_horz_motion(0, 0, 0),
    m_camera_pitch(0.0f),
    m_camera_yaw(0.0f),
    m_camera_pos(0, 0, 0)
{
}


// Set if the player is on solid ground or not.
// If so, reset their gravity to nothing.
void Player::setOnSolidGround(bool val)
{
    m_on_solid_ground = val;
    if (val) {
        m_vert_motion = MyVec4(0, 0, 0);
    }
}


// If the player is jumping upward, and hits the 
// ceiling, immediate reverse their vertical movement.
void Player::bounceOffCeiling()
{
    GLfloat y = m_vert_motion.y();
    if (y > 0) {
        m_vert_motion = MyVec4(0, -y, 0);
    }
}


// Update the player's position and bounding box.
void Player::setPlayerPos(const MyVec4 &pos) 
{
    m_player_pos = pos;
    
    GLfloat eye_level = static_cast<GLfloat>(PLAYER_EYE_LEVEL);
    MyVec4 eye_offset(0, eye_level, 0);
    m_camera_pos = pos.plus(eye_offset);

    GLfloat min_x = pos.x() - (PLAYER_WIDTH / 2);
    GLfloat max_x = pos.x() + (PLAYER_WIDTH / 2);
    GLfloat min_y = pos.y();
    GLfloat max_y = pos.y() + PLAYER_HEIGHT;
    GLfloat min_z = pos.z() - (PLAYER_WIDTH / 2);
    GLfloat max_z = pos.z() + (PLAYER_WIDTH / 2);

    MyVec4 lower(min_x, min_y, min_z);
    MyVec4 upper(max_x, max_y, max_z);
    m_bounding_box = MyBoundingBox(lower, upper);
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
    GLfloat elapsed_seconds = elapsed_msec / 1000.0f;

    GLfloat degrees_per_pixel = GetConfig().window.mouse_degrees_per_pixel;

    // Handle the mouse movement.
    // This is always instantaneous, and not tied to the elapsed time.
    m_camera_yaw   += static_cast<GLfloat>(msg.mouse_diff_x) * degrees_per_pixel;
    m_camera_pitch -= static_cast<GLfloat>(msg.mouse_diff_y) * degrees_per_pixel;
    clampRotations();

    // How to deal with movement depends on "noclip".
    if (GetConfig().debug.noclip) {
        calcNoclipMotion(elapsed_msec, msg);
    }
    else {
        calcStandardMotion(elapsed_msec, msg);
    }
}


// Calc the motion for "noclip" mode.
void Player::calcNoclipMotion(int elapsed_msec, const EventStateMsg &msg)
{
    // For noclip, kill any gravity through a reset.
    m_vert_motion = MyVec4(0, 0, 0);

    // Move around at run speed or flight speed.
    GLfloat speed = msg.speed_boost ?
        GetConfig().logic.player_flight_speed :
        GetConfig().logic.player_walk_speed;

    // Convert speed from m/sec to cm/msec.
    GLfloat scaled_speed = (speed * BLOCK_SCALE) / 1000.0f;

    // Rotation vectors.
    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);

    // Recalc and rotate the vertical motion. This is terms of cm/msec.
    MyVec4 vert_move(0, 0, 0);
    if (msg.move_fwd) {
        vert_move = VEC4_NORTHWARD;
    }
    else if (msg.move_bkwd) {
        vert_move = VEC4_SOUTHWARD;
    }

    if (msg.move_left) {
        vert_move = vert_move.plus(VEC4_WESTWARD);
    }
    else if (msg.move_right) {
        vert_move = vert_move.plus(VEC4_EASTWARD);
    }

    vert_move = vert_move.normalized().times(scaled_speed);
    m_horz_motion = roty.times(rotx).times(vert_move);

    // Recalc the horizontal motion. It doesn't get rotated. This in terms of cm/sec.
    MyVec4 horz_move(0, 0, 0);
    if (msg.move_up) {
        horz_move = VEC4_UPWARD;
    }
    else if (msg.move_down) {
        horz_move = VEC4_DOWNWARD;
    }

    m_vert_motion = horz_move.normalized().times(scaled_speed);

    // Apply the vertical and horizontal movement.
    MyVec4  player_pos = m_player_pos;
    GLfloat elapsed_msec_f = static_cast<GLfloat>(elapsed_msec);

    MyVec4 vert_in_cm = m_horz_motion.times(elapsed_msec_f);
    player_pos = MyMatrix4by4::Translate(vert_in_cm).times(player_pos);

    MyVec4 horz_in_cm = m_vert_motion.times(elapsed_msec_f);
    player_pos = MyMatrix4by4::Translate(horz_in_cm).times(player_pos);

    // Update both the player *and* camera positions.
    setPlayerPos(player_pos);
}


void Player::calcStandardMotion(int elapsed_msec, const EventStateMsg &msg)
{
    // If the player is jumping, add upward motion, then they're not on solid ground anymore.
    if (msg.jump && m_on_solid_ground) {
        GLfloat jump_speed = GetConfig().logic.player_jump_speed;
        GLfloat scaled_jump_speed = (jump_speed * BLOCK_SCALE) / 1000.0f;

        MyVec4 upward(0, scaled_jump_speed, 0);
        m_vert_motion = m_vert_motion.plus(upward);
        m_on_solid_ground = false;
    }

    // Move around at walk speed.
    GLfloat speed = msg.speed_boost ?
        GetConfig().logic.player_run_speed :
        GetConfig().logic.player_walk_speed;

    // Convert speed from m/sec to cm/msec.
    GLfloat scaled_speed = (speed * BLOCK_SCALE) / 1000.0f;

    // Rotation vectors.
    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);

    // Recalc and rotate the vertical motion. This is terms of cm/msec.
    // For standard motion, there's only vertical movement, no horizontal.
    MyVec4 vert_move(0, 0, 0);
    if (msg.move_fwd) {
        vert_move = VEC4_NORTHWARD;
    }
    else if (msg.move_bkwd) {
        vert_move = VEC4_SOUTHWARD;
    }

    if (msg.move_left) {
        vert_move = vert_move.plus(VEC4_WESTWARD);
    }
    else if (msg.move_right) {
        vert_move = vert_move.plus(VEC4_EASTWARD);
    }

    vert_move = vert_move.normalized().times(scaled_speed);
    m_horz_motion = roty.times(rotx).times(vert_move);

    // Add new gravity to the horizontal motion. Convert from m/sec2 to cm/msec2.
    GLfloat gravity = GetConfig().logic.player_gravity;
    GLfloat scaled_gravity = (gravity * BLOCK_SCALE) / (1000.0f * 1000.0f);
    MyVec4  add_to_gravity = VEC4_DOWNWARD.times(scaled_gravity * elapsed_msec);

    m_vert_motion = m_vert_motion.plus(add_to_gravity);

    // Apply the vertical and horizontal movement.
    MyVec4  player_pos = m_player_pos;
    GLfloat elapsed_msec_f = static_cast<GLfloat>(elapsed_msec);

    MyVec4 vert_in_cm = m_horz_motion.times(elapsed_msec_f);
    player_pos = MyMatrix4by4::Translate(vert_in_cm).times(player_pos);

    MyVec4 horz_in_cm = m_vert_motion.times(elapsed_msec_f);
    player_pos = MyMatrix4by4::Translate(horz_in_cm).times(player_pos);

    // Update both the player *and* camera positions.
    setPlayerPos(player_pos);
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


#include "stdafx.h"
#include "player.h"

#include "config.h"
#include "format.h"
#include "heads_up_display.h"


// The player's dimensions, in centimeters.
const int PLAYER_HEIGHT = 190;
const int PLAYER_WIDTH  =  90;
const int PLAYER_EYE_LEVEL = 160;


Player::Player(GameWorld &world) :
    m_game_world(world),
    m_on_solid_ground(false),
    m_gravity_vec(0, 0, 0),
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
        m_gravity_vec = MyVec4(0, 0, 0);
    }
}


// Set the player's position.
void Player::setPlayerPos(const MyVec4 &pos) 
{
    m_player_pos = pos;
    
    GLfloat eye_level = static_cast<GLfloat>(PLAYER_EYE_LEVEL);
    MyVec4 eye_offset(0, eye_level, 0);
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
    GLfloat elapsed_seconds = elapsed_msec / 1000.0f;

    GLfloat degrees_per_pixel = GetConfig().window.mouse_degrees_per_pixel;

    // Handle the mouse movement.
    // This is always instantaneous, and not tied to the elapsed time.
    int mouse_diff_x = msg.getMouseDiffX();
    int mouse_diff_y = msg.getMouseDiffY();
    m_camera_yaw   += static_cast<GLfloat>(mouse_diff_x) * degrees_per_pixel;
    m_camera_pitch -= static_cast<GLfloat>(mouse_diff_y) * degrees_per_pixel;
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
    // For noclip, kill any gravity.
    m_gravity_vec = MyVec4(0, 0, 0);

    // We will always move around at run speed.
    GLfloat speed = GetConfig().logic.player_walk_speed;
    if (msg.getSpeedBoost()) {
        speed *= 10;
    }

    // Convert speed from m/sec to cm/msec.
    GLfloat scaled_speed = (speed * BLOCK_SCALE) / 1000.0f;
    GLfloat centimeters  = scaled_speed * elapsed_msec;

    // Calc the vertical movement.
    MyVec4 vert_move(0, 0, 0);
    if (msg.getMoveFwd()) {
        vert_move = VEC4_NORTHWARD;
    }
    else if (msg.getMoveBkwd()) {
        vert_move = VEC4_SOUTHWARD;
    }

    if (msg.getMoveLeft()) {
        vert_move = vert_move.plus(VEC4_WESTWARD);
    }
    else if (msg.getMoveRight()) {
        vert_move = vert_move.plus(VEC4_EASTWARD);
    }

    vert_move = vert_move.normalized().times(centimeters);

    // Calc the horizontal movement.
    MyVec4 horz_move(0, 0, 0);
    if (msg.getMoveUp()) {
        horz_move = VEC4_UPWARD;
    }
    else if (msg.getMoveDown()) {
        horz_move = VEC4_DOWNWARD;
    }

    horz_move = horz_move.normalized().times(centimeters);

    // Rotation vectors.
    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);

    // Apply the vertical and horizontal movement.
    MyVec4 player_pos = m_player_pos;

    vert_move  = roty.times(rotx).times(vert_move);
    player_pos = MyMatrix4by4::Translate(vert_move).times(player_pos);

    player_pos = MyMatrix4by4::Translate(horz_move).times(player_pos);

    // Update both the player *and* camera positions.
    setPlayerPos(player_pos);
}


void Player::calcStandardMotion(int elapsed_msec, const EventStateMsg &msg)
{
    // Move around at walk speed.
    GLfloat speed = GetConfig().logic.player_walk_speed;
    if (msg.getSpeedBoost()) {
        speed = GetConfig().logic.player_run_speed;
    }

    // Convert speed from m/sec to cm/msec.
    GLfloat scaled_speed = (speed * BLOCK_SCALE) / 1000.0f;
    GLfloat centimeters  = scaled_speed * elapsed_msec;

    // Calc the vertical movement.
    MyVec4 vert_move(0, 0, 0);
    if (msg.getMoveFwd()) {
        vert_move = VEC4_NORTHWARD;
    }
    else if (msg.getMoveBkwd()) {
        vert_move = VEC4_SOUTHWARD;
    }

    if (msg.getMoveLeft()) {
        vert_move = vert_move.plus(VEC4_WESTWARD);
    }
    else if (msg.getMoveRight()) {
        vert_move = vert_move.plus(VEC4_EASTWARD);
    }

    vert_move = vert_move.normalized().times(centimeters);

    // Calc the horizontal movement.
    MyVec4 horz_move(0, 0, 0);
    if (msg.getMoveUp()) {
        horz_move = VEC4_UPWARD;
    }
    else if (msg.getMoveDown()) {
        horz_move = VEC4_DOWNWARD;
    }

    horz_move = horz_move.normalized().times(centimeters);

    // Rotation vectors.
    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);

    // Apply the vertical and horizontal movement.
    MyVec4 player_pos = m_player_pos;

    vert_move = roty.times(rotx).times(vert_move);
    player_pos = MyMatrix4by4::Translate(vert_move).times(player_pos);

    player_pos = MyMatrix4by4::Translate(horz_move).times(player_pos);

    // Add new gravity. Convert from m/sec2 to cm/msec2.
    GLfloat gravity = GetConfig().logic.player_gravity;
    GLfloat scaled_gravity = (gravity * BLOCK_SCALE) / (1000.0f * 1000.0f);
    MyVec4  add_to_gravity = VEC4_DOWNWARD.times(scaled_gravity * elapsed_msec);

    m_gravity_vec = m_gravity_vec.plus(add_to_gravity);
    SetDebugLine(fmt::format("Gravity: {0:.03f}, Centimeters: {1:.03f}", scaled_speed, centimeters));

    MyVec4 applied = m_gravity_vec.times(elapsed_msec);
    player_pos = MyMatrix4by4::Translate(applied).times(player_pos);

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

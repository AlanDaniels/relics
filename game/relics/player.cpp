
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
    m_vertical_vec(0, 0, 0),
    m_horizontal_vec(0, 0, 0),
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
        m_vertical_vec = MyVec4(0, 0, 0);
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

#if 0
    @@@ // TODO: CONTINUE HERE.

    // Handle the player movement.
    GLfloat meters_per_sec = GetConfig().logic.player_walk_speed;
    GLfloat cm_per_sec     = meters_per_sec * BLOCK_SCALE;
    GLfloat centimeters    = cm_per_sec * elapsed_seconds;

    if (msg.getSpeedBoost()) {
        centimeters *= 10.0f;
    }

    // Updat the vertical motion vec.
    MyVec4 vert_vec(0, 0, 0);

    if (msg.getMoveFwd()) {
        vert_vec = vert_vec.plus(VEC4_NORTHWARD);
    }
    else if (msg.getMoveBkwd()) {
        vert_vec = vert_vec.plus(VEC4_SOUTHWARD);
    }

    if (msg.getMoveLeft()) {
        vert_vec = vert_vec.plus(VEC4_WESTWARD);
    }
    else if (msg.getMoveRight()) {
        vert_vec = vert_vec.plus(VEC4_EASTWARD);
    }

    vert_vec = vert_vec.normalized();

    // Update the horizontal motion.
    MyVec4 horz_vec(0, 0, 0);

    if (msg.getMoveUp()) {
        horz_vec = horz_vec.plus(VEC4_UPWARD);
    }
    else if (msg.getMoveDown()) {
        horz_vec = horz_vec.plus(VEC4_DOWNWARD);
    }

    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);

    // Apply the vertical motion vector.
    MyMatrix4by4 vert_rotator = roty.times(rotx);
    MyVec4 vert_move = vert_rotator.times(vert_vec).times(centimeters);
    m_player_pos = MyMatrix4by4::Translate(vert_move).times(m_player_pos);

    // Rotate the motion vector and use it to move our position.
    // TODO: WORK ON HORZ.

    // Apply gravity.
    if (m_apply_gravity) {
        GLfloat grav_cm_per_sec2 = (GRAVITY_METERS_PER_SEC2 * BLOCK_SCALE);
        GLfloat grav_cm_per_msec = grav_cm_per_sec2 * (elapsed_msec / 1000.0f);

        m_gravity_vec = m_gravity_vec.plus(MyVec4(0, -grav_cm_per_msec, 0));

        MyMatrix4by4 tr = MyMatrix4by4::Translate(m_gravity_vec);
        m_player_pos = tr.times(m_player_pos);
    }

    // All done. Update the camera as well.
    MyVec4 eye_offset(0, PLAYER_EYE_LEVEL, 0);
    m_camera_pos = m_player_pos.plus(eye_offset);
#endif
}


// Calc the motion for "noclip" mode.
void Player::calcNoclipMotion(int elapsed_msec, const EventStateMsg &msg)
{
    // For noclip, kill any player momentum.
    m_horizontal_vec = MyVec4(0, 0, 0);
    m_vertical_vec   = MyVec4(0, 0, 0);

    // We will always move around at run speed.
    GLfloat speed_m_per_sec = GetConfig().logic.player_run_speed;
    GLfloat speed_cm_p_msec = (speed_m_per_sec * BLOCK_SCALE) / 1000.0f;
    GLfloat centimeters     = speed_cm_p_msec * elapsed_msec;

    SetDebugLine(fmt::format("Speed: {0:.03f}, Centimeters: {1:.03f}", speed_cm_p_msec, centimeters));

    // Calc the vertical movement.
    MyVec4 vert_direction(0, 0, 0);

    if (msg.getMoveFwd()) {
        vert_direction = vert_direction.plus(VEC4_NORTHWARD);
    }
    else if (msg.getMoveBkwd()) {
        vert_direction = vert_direction.plus(VEC4_SOUTHWARD);
    }

    if (msg.getMoveLeft()) {
        vert_direction = vert_direction.plus(VEC4_WESTWARD);
    }
    else if (msg.getMoveRight()) {
        vert_direction = vert_direction.plus(VEC4_EASTWARD);
    }

    vert_direction = vert_direction.normalized().times(centimeters);

    // Rotation vectors.
    MyMatrix4by4 roty = MyMatrix4by4::RotateY(m_camera_yaw);
    MyMatrix4by4 rotx = MyMatrix4by4::RotateX(-m_camera_pitch);

    // Apply the vertical movement.
    MyMatrix4by4 vert_rotate = roty.times(rotx);
    MyVec4 vert_move = vert_rotate.times(vert_direction);
    m_player_pos = MyMatrix4by4::Translate(vert_move).times(m_player_pos);

    // TODO: Apply Horz here.

#if 0
    // Calc the horizontal movement.
    MyVec4 horz_vec(0, 0, 0);

    if (msg.getMoveUp()) {
        horz_vec = horz_vec.plus(VEC4_UPWARD);
    }
    else if (msg.getMoveDown()) {
        horz_vec = horz_vec.plus(VEC4_DOWNWARD);
    }

    horz_vec = horz_vec.normalized();
#endif
}


void Player::calcStandardMotion(int elapsed_msec, const EventStateMsg &msg)
{
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

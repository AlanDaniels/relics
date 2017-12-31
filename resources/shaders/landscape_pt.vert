#version 400

// A basic shader for position, and one texuv.
layout (location = 0) in vec4 position;
layout (location = 4) in vec2 texuv;

uniform float near_plane;
uniform float far_plane;

uniform vec4  camera_pos;
uniform float camera_yaw;
uniform float camera_pitch;

out vec2  var_texuv;
out float var_dist; // Distance to the camera, but only in XZ.


// NOTE: Asking the card to calculate that View Frustum, EVERY TIME, adds up to
// a lot of unnecessary work in the video card. Instead, calculate it ahead of
// time, and feed it in as a uniform value.
mat4 view_frustum(float angle_of_view, float aspect_ratio, float z_near, float z_far) {
    return mat4(
        vec4(1.0 / tan(angle_of_view), 0.0, 0.0, 0.0),
        vec4(0.0, aspect_ratio / tan(angle_of_view),  0.0, 0.0),
        vec4(0.0, 0.0, (z_far + z_near) / (z_far - z_near), 1.0),
        vec4(0.0, 0.0, -2.0 * z_far * z_near / (z_far - z_near), 0.0));
}

mat4 scale(float x, float y, float z) {
    return mat4(
        vec4(x,   0.0, 0.0, 0.0),
        vec4(0.0, y,   0.0, 0.0),
        vec4(0.0, 0.0, z,   0.0),
        vec4(0.0, 0.0, 0.0, 1.0));
}

mat4 translate(float x, float y, float z) {
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(x,   y,   z,   1.0));
}

mat4 rotate_x(float r) {
    return mat4(
        vec4(1.0,     0.0,     0.0, 0.0),
        vec4(0.0,  cos(r),  sin(r), 0.0),
        vec4(0.0, -sin(r),  cos(r), 0.0),
        vec4(0.0,     0.0,     0.0, 1.0));
}

mat4 rotate_y(float r) {
    return mat4(
        vec4(cos(r), 0.0, -sin(r), 0.0),
        vec4(  0.0,  1.0,     0.0, 0.0),
        vec4(sin(r), 0.0,  cos(r), 0.0),
        vec4(   0.0, 0.0,     0.0, 1.0));
}

void main()
{
    float angle_of_view = radians(45.0);
    float aspect_ratio  = 1920.0 / 1080.0;
    gl_Position = view_frustum(angle_of_view, aspect_ratio, near_plane, far_plane)
        * rotate_x(radians( camera_pitch))
        * rotate_y(radians(-camera_yaw))
        * translate(-camera_pos.x, -camera_pos.y, -camera_pos.z)
        * position;

    var_texuv = texuv;
    var_dist  = distance(camera_pos.xz, position.xz);
}


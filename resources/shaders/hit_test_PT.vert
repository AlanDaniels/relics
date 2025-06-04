#version 330

// Our "hit test" shader.

layout (location = 0) in vec4 in_position;
layout (location = 4) in vec2 in_texuv;

uniform mat4  mat_frustum;
uniform vec4  camera_pos;
uniform float camera_yaw;
uniform float camera_pitch;

out vec2  var_texuv;
out float var_dist;  // Distance to the camera, but only in XZ.


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

    gl_Position = mat_frustum
        * rotate_x(radians( camera_pitch))
        * rotate_y(radians(-camera_yaw))
        * translate(-camera_pos.x, -camera_pos.y, -camera_pos.z)
        * in_position;

    var_texuv = in_texuv;
    var_dist  = distance(camera_pos.xz, in_position.xz);
}


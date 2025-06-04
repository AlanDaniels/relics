#version 330

// Our vert shader for the skybox.

// A basic shader for position, and one texuv.
layout (location = 0) in vec4 in_position;


uniform mat4  mat_frustum;
uniform mat4  mat_frustum_rotate;


out vec3 var_tex_coords;


void main()
{
    float angle_of_view = radians(45.0);
    float aspect_ratio  = 1920.0 / 1080.0;

    gl_Position = mat_frustum_rotate * in_position;

    vec4 result = mat_frustum * in_position;

    var_tex_coords = result.xyz;
}


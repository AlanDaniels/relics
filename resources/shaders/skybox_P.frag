#version 330

// Our frag shader for the skybox.

uniform samplerCube textures[1];


in vec3 var_tex_coords;


layout(location = 0) out vec4 FragColor;


void main() {
    FragColor = texture(textures[0], var_tex_coords);
}


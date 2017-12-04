#version 330

// A basic shader for position, and one texuv.
uniform sampler2D textures[1];

uniform float fade_distance;
uniform float draw_distance;

in vec2  var_texuv;
in float var_dist;


layout(location = 0) out vec4 FragColor;


void main() {
    float fade_start = draw_distance - fade_distance;

    vec4 tex_color = texture2D(textures[0], var_texuv); 

    // For fragments near enough, draw them normally.
    if (var_dist < fade_start) {
        FragColor = tex_color;
    }

    // For ones too far away, don't draw them at all.
    else if (var_dist > draw_distance) {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }

    // For in-between, fade between normal and nothingness.
    else {
        float mix_factor = (var_dist - fade_start) / draw_distance;
        vec4 tex_color = tex_color;
        FragColor = mix(tex_color, vec4(tex_color.rgb, 0.0), mix_factor);
    }
}


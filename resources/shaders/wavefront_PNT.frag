#version 330

// Our "landscape" frag shader.

uniform float     fade_distance;
uniform float     draw_distance;
uniform sampler2D textures[1];


in vec2  var_texuv;
in float var_incident;
in float var_dist;


layout(location = 0) out vec4 FragColor;


void main() {
    float fade_start = draw_distance - fade_distance;

    // 50 percent seems like a suitable amount of the effect.
    float fresnel = 0.5 * (1 - var_incident);

    // Darken things a bit when we're not looking at them dead-on.
    vec4 tex_color = mix(
        texture2D(textures[0], var_texuv),
        vec4(0, 0, 0, 1),
        fresnel);

    // For fragments near enough, draw them normally.
    if (var_dist < fade_start) {
        FragColor = tex_color;
    }

    // For ones too far away, don't draw them at all.
    else if (var_dist > draw_distance) {
        FragColor = vec4(0, 0, 0, 0);
    }

    // For in-between, fade between normal and nothingness.
    else {
        float mix_factor = (var_dist - fade_start) / fade_distance;
        vec4 tex_color = tex_color;
        FragColor = mix(tex_color, vec4(tex_color.rgb, 0.0), mix_factor);
    }
}


#version 410

in vec2 texture_coords;

uniform sampler2D sprite;
uniform float offsetx;
uniform float offsety;

uniform float weight;
uniform bool applyOffset;  // Nova variável uniforme

out vec4 frag_color; 

void main () {
    vec2 coords = texture_coords;

    // Aplica o deslocamento se applyOffset for true
    if (applyOffset) {
        coords.x += offsetx;
        coords.y += offsety;
    }

    vec4 texel = mix(texture(sprite, coords), vec4(0, 0, 1, 1), weight);
    
    if (texel.a < 0.5) {
        discard;
    }

    frag_color = texel;
}
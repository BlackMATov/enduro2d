uniform sampler2D u_texture;

uniform float u_glyph_dilate;
uniform float u_outline_width;
uniform vec4 u_outline_color;

varying vec2 v_st0;
varying vec4 v_color0;

void main() {
    vec4 t = texture2D(u_texture, v_st0);

    float glyph_alpha = t.a;
    float outline_alpha = t.r;

    vec4 glyph_color = vec4(v_color0.rgb * v_color0.a, v_color0.a);
    vec4 outline_color = vec4(u_outline_color.rgb * u_outline_color.a, u_outline_color.a);

    vec2 layers_mask = vec2(
        step(0.0001, v_color0.a),
        step(0.0001, u_outline_color.a * u_outline_width));

    gl_FragColor =
        layers_mask.x * glyph_alpha * glyph_color +
        layers_mask.y * outline_alpha * outline_color * (1.0 - glyph_alpha);
}

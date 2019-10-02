#ifdef GL_OES_standard_derivatives
#  extension GL_OES_standard_derivatives : require
#endif

uniform sampler2D u_texture;

uniform float u_glyph_dilate;
uniform float u_outline_width;
uniform vec4 u_outline_color;

varying vec2 v_st0;
varying vec4 v_color0;

void main() {
    float distance = texture2D(u_texture, v_st0).a;
    float width = fwidth(distance);

    float glyph_center = 0.5 - u_glyph_dilate * 0.5;
    float outline_center = glyph_center - u_outline_width * 0.5;

    float glyph_alpha = smoothstep(glyph_center - width, glyph_center + width, distance);
    float outline_alpha = smoothstep(outline_center - width, outline_center + width, distance);

    vec4 glyph_color = vec4(v_color0.rgb * v_color0.a, v_color0.a);
    vec4 outline_color = vec4(u_outline_color.rgb * u_outline_color.a, u_outline_color.a);

    vec2 layers_mask = vec2(
        step(0.0001, v_color0.a),
        step(0.0001, u_outline_color.a * u_outline_width));

    gl_FragColor =
        layers_mask.x * glyph_alpha * glyph_color +
        layers_mask.y * outline_alpha * outline_color * (1.0 - glyph_alpha);
}

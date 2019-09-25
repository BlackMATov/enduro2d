uniform sampler2D u_texture;

varying vec2 v_st0;
varying vec4 v_color0;

void main() {
    vec4 c = texture2D(u_texture, v_st0) * v_color0;
    c.rgb *= c.a;
    gl_FragColor = c;
}

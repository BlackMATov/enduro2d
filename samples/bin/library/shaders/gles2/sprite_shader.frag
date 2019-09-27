uniform sampler2D u_texture;

varying vec4 v_tint;
varying vec2 v_st;

void main() {
    vec2 st = vec2(v_st.s, 1.0 - v_st.t);
    gl_FragColor = texture2D(u_texture, st) * v_tint;
}

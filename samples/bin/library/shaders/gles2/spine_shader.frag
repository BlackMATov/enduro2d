uniform sampler2D u_texture;

varying vec4 v_tint;
varying vec2 v_st;

void main() {
    gl_FragColor = texture2D(u_texture, v_st) * v_tint;
}

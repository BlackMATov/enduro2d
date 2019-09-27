uniform sampler2D u_texture;

in vec4 v_tint;
in vec2 v_st;

void main() {
    gl_FragColor = texture(u_texture, v_st) * v_tint;
}

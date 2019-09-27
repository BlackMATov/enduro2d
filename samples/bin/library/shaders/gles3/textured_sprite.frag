uniform sampler2D u_texture;

in vec4 v_color;
in vec2 v_uv;

void main(){
    gl_FragColor = v_color * texture(u_texture, v_uv);
}

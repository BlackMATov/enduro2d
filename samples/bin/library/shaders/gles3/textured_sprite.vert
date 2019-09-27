in vec2 a_position;
in vec2 a_uv;
in vec4 a_color;

layout(std140) uniform cb_pass {
    mat4 u_matrix_vp;
};

out vec4 v_color;
out vec2 v_uv;

void main(){
    v_color = a_color;
    v_uv = a_uv;
    gl_Position = vec4(a_position, 0.0, 1.0) * u_matrix_vp;
}

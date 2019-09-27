in vec3 a_vertex;
in vec4 a_tint;
in vec2 a_st;

layout(std140) uniform cb_pass {
    mat4 u_matrix_vp;
};

out vec4 v_tint;
out vec2 v_st;

void main() {
    v_st = a_st;
    v_tint = a_tint;
    gl_Position = vec4(a_vertex, 1.0) * u_matrix_vp;
}

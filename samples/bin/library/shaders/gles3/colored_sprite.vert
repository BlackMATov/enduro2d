in vec3 a_position;
in vec4 a_color;

layout(std140) uniform cb_pass {
    mat4 u_matrix_vp;
};

out vec4 v_color;

void main(){
    v_color = a_color;
    gl_Position = vec4(a_position, 1.0) * u_matrix_vp;
}

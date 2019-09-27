attribute vec3 a_vertex;
attribute vec2 a_st0;

uniform vec4 cb_pass[4];
#define u_matrix_vp mat4(cb_pass[0], cb_pass[1], cb_pass[2], cb_pass[3])

uniform vec4 cb_command[4];
#define u_matrix_m mat4(cb_command[0], cb_command[1], cb_command[2], cb_command[3])

varying vec2 v_st0;

void main() {
    v_st0 = a_st0;
    gl_Position = vec4(a_vertex, 1.0) * u_matrix_m * u_matrix_vp;
}

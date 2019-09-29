attribute vec3 a_vertex;
attribute vec2 a_st0;

uniform vec4 cb_pass[5];
#define u_matrix_vp mat4(cb_pass[1], cb_pass[2], cb_pass[3], cb_pass[4])
#define u_screen_s cb_pass[0].xy

uniform vec4 cb_command[4];
#define u_matrix_m mat4(cb_command[0], cb_command[1], cb_command[2], cb_command[3])

varying vec2 v_st0;

vec4 vertex_to_homo(vec3 pos) {
    return vec4(pos, 1.0) * u_matrix_m * u_matrix_vp;
}

void main() {
    v_st0 = vec2(a_st0.s, 1.0 - a_st0.t);
    gl_Position = vertex_to_homo(a_vertex);
}

attribute vec2 a_position;
attribute vec2 a_uv;
attribute vec4 a_color;

uniform vec4 cb_pass[4];
#define u_matrix_vp mat4(cb_pass[0], cb_pass[1], cb_pass[2], cb_pass[3])

varying vec4 v_color;
varying vec2 v_uv;

void main(){
    v_color = a_color;
    v_uv = a_uv;
    gl_Position = vec4(a_position, 0.0, 1.0) * u_matrix_vp;
}

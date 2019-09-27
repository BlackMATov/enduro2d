attribute vec3 a_position;
attribute vec4 a_color;

uniform vec4 cb_pass[5];
#define u_matrix_vp mat4(cb_pass[1], cb_pass[2], cb_pass[3], cb_pass[4])
#define u_screen_s cb_pass[0].xy

varying vec4 v_color;

void main(){
    v_color = a_color;
    gl_Position = vec4(a_position, 1.0) * u_matrix_vp;
}

uniform vec2 u_screen_s;
uniform mat4 u_matrix_m;
uniform mat4 u_matrix_vp;

attribute vec3 a_vertex;
attribute vec2 a_st0;
attribute vec4 a_color0;

varying vec2 v_st0;
varying vec4 v_color0;

#define VERTEX_SNAPPING_ON

vec2 round(vec2 v) {
    return vec2(
        floor(v.x + 0.5),
        floor(v.y + 0.5));
}

vec4 pixel_snap(vec4 pos) {
    vec2 hpc = u_screen_s * 0.5;
    vec2 pixel_pos = round((pos.xy / pos.w) * hpc);
    pos.xy = pixel_pos / hpc * pos.w;
    return pos;
}

vec4 vertex_to_homo(vec3 pos) {
    return vec4(pos, 1.0) * u_matrix_m * u_matrix_vp;
}

void main() {
    v_st0 = vec2(a_st0.s, 1.0 - a_st0.t);
    v_color0 = a_color0;
#ifndef VERTEX_SNAPPING_ON
    gl_Position = vertex_to_homo(a_vertex);
#else
    gl_Position = pixel_snap(vertex_to_homo(a_vertex));
#endif
}

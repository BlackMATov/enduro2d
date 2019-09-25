uniform mat4 u_matrix_m;
uniform mat4 u_matrix_vp;

attribute vec3 a_vertex;
attribute vec2 a_st0;

varying vec2 v_st0;

vec4 vertex_to_homo(vec3 pos) {
    return vec4(pos, 1.0) * u_matrix_m * u_matrix_vp;
}

void main() {
    v_st0 = vec2(a_st0.s, 1.0 - a_st0.t);
    gl_Position = vertex_to_homo(a_vertex);
}

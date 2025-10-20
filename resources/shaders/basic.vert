#version 410 core
layout(location=0) in vec3 a_pos;
uniform mat4 u_mvp;
uniform float u_pointSize;
uniform vec4 u_clipPlane; // world-space plane (n,xyz, d)

void main(){
    gl_Position = u_mvp * vec4(a_pos, 1.0);
    gl_PointSize = u_pointSize;
    gl_ClipDistance[0] = dot(vec4(a_pos, 1.0), u_clipPlane);
}

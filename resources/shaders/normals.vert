#version 410 core
layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_normal;

uniform vec4 u_clipPlane;
// Pass model-space data to geometry stage
out vec3 v_pos;
out vec3 v_normal;
out float clip;

// We set gl_Position but geometry shader will recompute with u_mvp
void main() {
    v_pos = a_pos;
    v_normal = a_normal;
    clip = dot(u_clipPlane, vec4(a_pos, 1.0));
    gl_Position = vec4(a_pos, 1.0);
}


#version 410 core
layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 u_mvp;
uniform float u_normalLen; // length in world units

in vec3 v_pos[];
in vec3 v_normal[];

out vec3 g_color; // pass to frag if needed; but we'll use uniform color instead

void main() {
    vec3 p0 = v_pos[0];
    float lenN = length(v_normal[0]);
    if (lenN < 1e-8) {
        // Skip zero-length normals
        return;
    }
    vec3 n = normalize(v_normal[0]);
    vec3 p1 = p0 + n * u_normalLen;

    gl_Position = u_mvp * vec4(p0, 1.0);
    EmitVertex();

    gl_Position = u_mvp * vec4(p1, 1.0);
    EmitVertex();

    EndPrimitive();
}

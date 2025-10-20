#version 410 core
layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 u_mvp;
uniform float u_normalLen; // length in world units
uniform vec4 u_clipPlane;  // clipping plane

in vec3 v_pos[];
in vec3 v_normal[];

void main() {
    vec3 p0 = v_pos[0];
    float lenN = length(v_normal[0]);
    if (lenN < 1e-8) {
        return;
    }
    vec3 n = normalize(v_normal[0]);
    vec3 p1 = p0 + n * u_normalLen;


    // Base point clip value; if it's outside, skip emitting this normal entirely.
    float baseClip = dot(u_clipPlane, vec4(p0, 1.0));
    if (baseClip < 0.0) {
        return;
    }

    gl_Position = u_mvp * vec4(p0, 1.0);
    gl_ClipDistance[0] = baseClip;
    EmitVertex();

    gl_Position = u_mvp * vec4(p1, 1.0);
    gl_ClipDistance[0] = baseClip;
    EmitVertex();

    EndPrimitive();
}

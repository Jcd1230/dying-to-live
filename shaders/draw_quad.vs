#version 300 es
layout(location = 0) in vec3 vp_modelspace;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_tangent;
layout(location = 3) in vec3 v_uv;

uniform float time;
out vec2 screen_pos;

void main()
{
	screen_pos = v_uv.xy;
	lowp vec4 v = vec4(vp_modelspace, 1.0);
	gl_Position = v;
}

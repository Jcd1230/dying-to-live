#version 330

precision highp float;

layout(location = 0) in vec3 vp_modelspace;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_tangent;
layout(location = 3) in vec3 v_uv;

struct light {
	vec3 pos;
	vec3 color;
	float range;
};

uniform mat4 MVP;
uniform float time;

out vec3 vertex_normal;
out vec3 vertex_tangent;
out vec3 vertex_bitangent;
out vec2 vertex_uv;
out float frag_depth;
out vec3 screen_pos;
out vec3 world_pos;

void main()
{
	light l1;
	l1.pos = vec3(5.0);
	l1.color = vec3(0.1,0.2,1.0);
	l1.range = 10.0;
	
	highp vec4 v = vec4(vp_modelspace, 1.0);
	vec4 vpos = MVP*v;	
	
	vertex_normal = v_normal;
	vertex_tangent = v_tangent;
	vertex_uv = v_uv.xy;
	
	screen_pos = vpos.xyz;
	//frag_depth = vpos.z/vpos.w;
	frag_depth = vpos.z;
	world_pos = vp_modelspace.xyz;
	
	gl_Position = vpos;
}

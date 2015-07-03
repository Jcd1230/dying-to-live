#version 300 es

uniform sampler2D diffuse;
uniform sampler2D normal;

uniform highp float time;
uniform highp mat4 MVP;
uniform highp mat4 MVPInv;

precision highp float;

in highp vec3 vertex_normal;
in highp vec3 vertex_tangent;
in highp vec3 vertex_bitangent;
in highp vec2 vertex_uv;
in highp float frag_depth;
in highp vec3 screen_pos;
in highp vec3 world_pos;

layout(location = 0) out highp vec3 renderColor;
layout(location = 1) out highp vec3 renderNormal;
layout(location = 2) out highp vec3 renderSpecular;
layout(location = 3) out highp float renderDepth;

void main()
{
	vec3 albedo = texture(diffuse, vertex_uv).rgb;
	
	vec3 tangent = normalize(vertex_tangent - dot(vertex_tangent, vertex_normal) * vertex_normal);
	vec3 bitangent = cross(tangent, vertex_normal);
	vec3 bump = texture(normal, vertex_uv).rgb * 2.0 - vec3(1.0);
	vec3 mappedNormal = normalize(mat3(tangent, bitangent, vertex_normal) * bump);
	mappedNormal = mix(vertex_normal, mappedNormal, 0.75); //0.75 = normal map "strength" (1.0, full affect, 0.0, use vertex normal)
	renderColor = albedo;
	renderNormal = (mappedNormal*0.5) + vec3(0.5);
	renderSpecular = vec3(0.0);
	renderDepth = frag_depth;
	
	return;
}
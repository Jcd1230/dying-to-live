#version 330

uniform sampler2D diffuse_t;
uniform sampler2D normal_t;
uniform sampler2D material_t;

uniform highp float time;

precision highp float;

in highp vec3 vertex_normal;
in highp vec3 vertex_tangent;
in highp vec3 vertex_bitangent;
in highp vec2 vertex_uv;
in highp float frag_depth;
in highp vec3 screen_pos;
in highp vec3 view_pos;

layout(location = 0) out highp vec3 renderColor;
layout(location = 1) out highp vec3 renderNormal;
layout(location = 2) out highp vec3 renderMaterial;
layout(location = 3) out highp float renderDepth;

// Default  Encoding

vec3 encode(vec3 normal) {
	return (normal*0.5) + vec3(0.5);
}


// Spherical Encoding
/*
vec3 encode(vec3 normal) {
	float f = sqrt(8*normal.z + 8);
	return vec3(normal.xy/f + 0.5, 0.0);
}
*/

void main()
{
	vec3 albedo = texture(diffuse_t, vertex_uv).rgb;
	vec3 material = texture(material_t, vertex_uv).rgb;
	
	vec3 tangent = normalize(vertex_tangent - dot(vertex_tangent, vertex_normal) * vertex_normal);
	vec3 bitangent = cross(tangent, vertex_normal);
	vec3 bump = texture(normal_t, vertex_uv).rgb * 2.0 - vec3(1.0);
	vec3 mappedNormal = normalize(mat3(tangent, bitangent, vertex_normal) * bump);
	mappedNormal = mix(vertex_normal, mappedNormal,  0.75); //0.75 = normal map "strength" (1.0, full affect, 0.0, use vertex normal)
	//mappedNormal = vertex_normal;
	renderColor = albedo;
	renderNormal = encode(mappedNormal);
	renderMaterial = material;
	renderDepth = -view_pos.z;
	return;
}
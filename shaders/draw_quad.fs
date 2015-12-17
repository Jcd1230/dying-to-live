#version 330

struct light {
	vec3 pos;
	vec3 color;
	float energy;
};

uniform highp mat4 VP;
uniform highp mat4 P;
uniform highp mat4 V;
uniform highp mat4 VPInv;
uniform highp float time;
uniform highp float max_specular_intensity;

uniform highp float vFOV;
uniform highp float hFOV;

uniform highp vec3 camera_world_pos;
uniform highp vec3 camera_forward;

uniform sampler2D renderColor;
uniform sampler2D renderNormal;
uniform sampler2D renderMaterial;
uniform sampler2D renderDepth;

precision highp float;

const float PI = 3.1415926;

in highp vec2 screen_pos;

layout(location = 0) out highp vec3 color;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i); 
	vec4 p = permute( permute( permute( 
			 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		   + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
		   + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
							dot(p2,x2), dot(p3,x3) ) );
}


/*
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
/*/

float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; 
}

float permute(float x) {
     return mod289(((x*34.0)+1.0)*x);
}

float taylorInvSqrt(float r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip)
{
	const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
	vec4 p,s;

	p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
	p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
	s = vec4(lessThan(p, vec4(0.0)));
	p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

	return p;
}
					
#define F4 0.309016994374947451
float snoise(vec4 v)
{
	const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
						0.276393202250021,  // 2 * G4
						0.414589803375032,  // 3 * G4
					   -0.447213595499958); // -1 + 4 * G4

	// First corner
	vec4 i  = floor(v + dot(v, vec4(F4)) );
	vec4 x0 = v -   i + dot(i, C.xxxx);

	// Other corners

	// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
	vec4 i0;
	vec3 isX = step( x0.yzw, x0.xxx );
	vec3 isYZ = step( x0.zww, x0.yyz );
	//  i0.x = dot( isX, vec3( 1.0 ) );
	i0.x = isX.x + isX.y + isX.z;
	i0.yzw = 1.0 - isX;
	//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
	i0.y += isYZ.x + isYZ.y;
	i0.zw += 1.0 - isYZ.xy;
	i0.z += isYZ.z;
	i0.w += 1.0 - isYZ.z;

	// i0 now contains the unique values 0,1,2,3 in each channel
	vec4 i3 = clamp( i0, 0.0, 1.0 );
	vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
	vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

	//  x0 = x0 - 0.0 + 0.0 * C.xxxx
	//  x1 = x0 - i1  + 1.0 * C.xxxx
	//  x2 = x0 - i2  + 2.0 * C.xxxx
	//  x3 = x0 - i3  + 3.0 * C.xxxx
	//  x4 = x0 - 1.0 + 4.0 * C.xxxx
	vec4 x1 = x0 - i1 + C.xxxx;
	vec4 x2 = x0 - i2 + C.yyyy;
	vec4 x3 = x0 - i3 + C.zzzz;
	vec4 x4 = x0 + C.wwww;

	// Permutations
	i = mod289(i); 
	float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
	vec4 j1 = permute( permute( permute( permute (
			 i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
		   + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
		   + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
		   + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));

	// Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
	// 7*7*6 = 294, which is close to the ring size 17*17 = 289.
	vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

	vec4 p0 = grad4(j0,   ip);
	vec4 p1 = grad4(j1.x, ip);
	vec4 p2 = grad4(j1.y, ip);
	vec4 p3 = grad4(j1.z, ip);
	vec4 p4 = grad4(j1.w, ip);

	// Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;
	p4 *= taylorInvSqrt(dot(p4,p4));

	// Mix contributions from the five corners
	vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
	vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
	m0 = m0 * m0;
	m1 = m1 * m1;
	return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
			   + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

}

vec3 OrenNayar(
	float roughness,
	vec3 albedo,
	vec3 lightDirection,
	vec3 viewDirection,
	vec3 surfaceNormal) 
{

	float LdotV = dot(lightDirection, viewDirection);
	float NdotL = dot(lightDirection, surfaceNormal);
	float NdotV = dot(surfaceNormal, viewDirection);

	float s = LdotV - NdotL * NdotV;
	float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

	float sigma2 = roughness * roughness;
	vec3 A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
	float B = 0.45 * sigma2 / (sigma2 + 0.09);

	if (NdotL < 0.0) {
		return vec3(1.0,0.0,0.0);
	}
	return (albedo) * max(0.0, NdotL) * (A + vec3(B * s / t)) / PI;
}

float gaussianSpecular(
  vec3 lightDirection,
  vec3 viewDirection,
  vec3 surfaceNormal,
  float shininess) {
  vec3 H = normalize(lightDirection + viewDirection);
  float theta = acos(dot(H, surfaceNormal));
  float w = theta / shininess;
  return exp(-w*w);
}

float beckmannDistribution(float x, float roughness) {
  float NdotH = max(x, 0.0001);
  float cos2Alpha = NdotH * NdotH;
  float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;
  float roughness2 = roughness * roughness;
  float denom = 3.141592653589793 * roughness2 * cos2Alpha * cos2Alpha;
  return exp(tan2Alpha / roughness2) / denom;
}

float cookTorranceSpecular(
  vec3 lightDirection,
  vec3 viewDirection,
  vec3 surfaceNormal,
  float roughness,
  float fresnel) {

  float VdotN = max(dot(viewDirection, surfaceNormal), 0.0);
  float LdotN = max(dot(lightDirection, surfaceNormal), 0.0);

  //Half angle vector
  vec3 H = normalize(lightDirection + viewDirection);

  //Geometric term
  float NdotH = max(dot(surfaceNormal, H), 0.0);
  float VdotH = max(dot(viewDirection, H), 0.000001);
  float LdotH = max(dot(lightDirection, H), 0.000001);
  float G1 = (2.0 * NdotH * VdotN) / VdotH;
  float G2 = (2.0 * NdotH * LdotN) / LdotH;
  float G = min(1.0, min(G1, G2));
  
  //Distribution term
  float D = beckmannDistribution(NdotH, roughness);

  //Fresnel term
  float F = pow(1.0 - VdotN, fresnel);

  //Multiply terms and done
  return  G * F * D / max(3.14159265 * VdotN, 0.000001);
}

// Default normal decode

vec3 decode(vec3 encoded) {
	return normalize((encoded - vec3(0.5))*2.0);
} 


// Spherically encoded normal
/*
vec3 decode(vec3 encoded) {
	vec2 fenc = encoded.xy*4-2;
	float f = dot(fenc, fenc);
	float g = sqrt(1-f/4);
	vec3 n;
	n.xy = fenc*g;
	n.z = 1-f/2;
	return n;
} 
*/
void main() //WARD + OREN NAYAR WIP
{
	vec2 uv = screen_pos;
	uv = mod(screen_pos, vec2(0.5,0.5)) * 2.0; //QUAD VIEW

	float depth = texture(renderDepth, uv).r;

	float zFar = 50.0; // TODO: CONVERT TO UNIFORM
	float zNear = 0.1; // ^
	float dist = zNear + depth*(zFar-zNear);
	vec2 ray = (uv*2.0) - vec2(1.0);
	vec3 cam_forward = normalize(camera_forward);
	vec3 cam_right = normalize(cross(cam_forward, vec3(0.0,1.0,0.0)));
	vec3 cam_up = normalize(cross(cam_forward, cam_right));
	vec3 world_pos = camera_world_pos;
	world_pos += normalize(cam_forward) * dist;

	float cam_top = dist*tan(vFOV);
	vec2 cam_topright = vec2(cam_top*16.0/9.0, cam_top);
	vec2 cam_ray = (uv*2.0 - vec2(1.0)) * cam_topright;
	world_pos += cam_right*(cam_ray.x);
	world_pos += cam_up*(-cam_ray.y);
	
	
	//Texture accesses
	vec3 albedo = texture(renderColor, uv).rgb;
	vec3 normalColor = texture(renderNormal, uv).rgb;
	vec3 material = texture(renderMaterial, uv).rgb;

	float specular_intensity = max(0.01, max_specular_intensity * material.r);
	float specular_weight = material.r;

	vec3 normal = decode(normalColor);

	float hasObject = sign(dot(normalColor,normalColor));

	//Lighting
	int n_lights = 2;
	light lights[2];
	lights[0].pos = camera_world_pos;
	lights[0].color = vec3(1.0, 1.0, 1.0);
	lights[0].energy = 100.0;
	lights[1].pos = vec3(5.0, 5.0, 5.0);
	lights[1].color = vec3(0.0, 1.0, 1.0);
	lights[1].energy = 0.0; 

	float roughness = 0.5;
	float fresnel = 1.6;
	vec3 final = vec3(0.0);
	vec3 viewDirection = normalize(camera_world_pos - world_pos); //direction to camera from frag
	vec3 diffuse = vec3(0.0);
	float specular = 0.0;
	
	vec3 hangingPoint = vec3(0.0, 20.0, 0.0);
	float cosLightSpread = -1.0;
	
	vec3 upleft;
	
	for (int i = 0; i < n_lights; i++) {
		vec3 lightDirection = normalize(lights[i].pos - world_pos); //direction to light
		vec3 TdirectionOfLamp = normalize(lights[i].pos - hangingPoint);
		if (dot(-lightDirection, TdirectionOfLamp) > cosLightSpread) {
			float dist_to_light = length(lights[i].pos - world_pos);
			float energy = lights[i].energy/(dist_to_light);
			
			float ctSpec = cookTorranceSpecular
			(
				lightDirection,
				viewDirection,
				normal,
				roughness,
				fresnel
			);

			float remainingEnergy = max(0.0, energy - specular);
			
			vec3 ONDiffuse = OrenNayar
			(
				roughness,
				albedo,
				lightDirection,
				viewDirection,
				normal
			);			

			diffuse += ONDiffuse * remainingEnergy * lights[i].color;
			specular += ctSpec * energy;
		}
	}
	
	final = diffuse + vec3(specular);
	final *= hasObject;
	final = final/(final + vec3(1.0));
	
	upleft = vec3(specular) * hasObject;
	
	
	//upleft = normal;
	// QUAD SPLIT VIEW
	color =  (diffuse * hasObject) * (1.0-step(0.5, screen_pos.y)) * (1.0 - step(0.5, screen_pos.x)); //LOWER LEFT
	color += (normalColor) * (1.0 - step(0.5, screen_pos.y)) * step(0.5, screen_pos.x); //LOWER RIGHT
	color += (upleft) * step(0.5, screen_pos.y) * (1.0 - step(0.5, screen_pos.x)); //UPPER LEFT
	color += (final) * step(0.5, screen_pos.y) * step(0.5, screen_pos.x); //UPPER RIGHT

	return;
}
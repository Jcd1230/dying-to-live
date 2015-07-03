#version 300 es 

uniform highp mat4 MVP;
uniform highp mat4 MV;
uniform highp mat4 VP;
uniform highp mat4 P;
uniform highp mat4 V;
uniform highp mat4 VPInv;
uniform highp mat4 MVPInv;
uniform highp mat3 MVPInvT;
uniform highp float time;

uniform highp vec3 camera_world_pos;

uniform sampler2D renderColor;
uniform sampler2D renderNormal;
uniform sampler2D renderSpecular;
uniform sampler2D renderDepth;

precision highp float;

in highp vec2 screen_pos;
layout(location = 0) out highp vec3 color;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

struct light {
	vec3 pos;
	vec3 color;
	float range;
};

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
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

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


void main()
{
	vec2 uv = screen_pos;
	uv = mod(screen_pos, vec2(0.5,0.5)) * 2.0; //QUAD VIEW
	//uv = screen_pos; // NORMAL
	
	float depth = texture(renderDepth, uv).r;
	vec4 NDC = vec4(1.0);
	NDC.x = uv.x*2.0 - 1.0;
	NDC.y = uv.y*2.0 - 1.0;
	NDC.z = depth;
	NDC.w = 1.0;
	vec4 CLIP = MVPInv * NDC;
	vec3 world_pos = CLIP.xyz/CLIP.w;
	
	vec3 normalColor = texture(renderNormal, uv).rgb;
	vec3 normal = normalize((normalColor - vec3(0.5))*2.0);
	
	float hasObject = step(0.01, dot(normalColor, normalColor));
	
	//Lighting
	light l1;
	l1.pos = vec3(10.0*cos(time), 20.0, 10.0*sin(time));
	l1.color = vec3(1.0);
	l1.range = 10.0;
	vec3 light_to_frag = normalize(l1.pos - world_pos);
	vec3 frag_to_cam = normalize(world_pos - camera_world_pos);
	vec3 albedo = texture(renderColor, uv).rgb;
	vec3 ambient = vec3(0.05) * albedo;
	vec3 diffuse = clamp(dot(normal,light_to_frag),0.0,1.0) * albedo;
	
	vec3 halfAngle = reflect(light_to_frag, normal);
	float specularAmt = clamp(dot(frag_to_cam, halfAngle), 0.0, 1.0);
	specularAmt = pow(specularAmt, 100.0);
	vec3 specular = vec3(specularAmt);
	//vec3 light_one = vec3(1.0,1.0,1.0);

	//vec3 final = vec3(0.5+0.5*snoise(vec4(world_pos, time)))*step(0.01, dot(normalColor,normalColor)); //vec3(dot(normal, normalize(world_pos - l1.pos)));
	
	vec3 final = clamp(diffuse, ambient, vec3(1.0)) + specular;
	final *= hasObject;
	//vec3 upleft = vec3(NDC.xyz);
	//upleft *= step(0.01, dot(normalColor,normalColor));
	vec3 upleft = specular * hasObject;
	//normalColor = vec3(abs(upleft-diffuse.b));
	// QUAD SPLIT VIEW
	color =  (diffuse) * (1.0-step(0.5, screen_pos.y)) * (1.0 - step(0.5, screen_pos.x)); //LOWER LEFT
	color += (normalColor) * (1.0 - step(0.5, screen_pos.y)) * step(0.5, screen_pos.x); //LOWER RIGHT
	color += (upleft) * step(0.5, screen_pos.y) * (1.0 - step(0.5, screen_pos.x)); //UPPER LEFT
	color += (final) * step(0.5, screen_pos.y) * step(0.5, screen_pos.x); //UPPER RIGHT

	return;
}
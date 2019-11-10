#version 430 core 
layout (quads, equal_spacing, ccw) in;

layout(location = 1) uniform float time = 0.0;

layout(location = 4) uniform int shader_mode = 0;
layout(location = 5) uniform float params[10];

struct pnQuad
{
	vec3 b0, b1, b2, b3;
	vec3 n0, n1, n2, n3;
	vec3 m0, m1, m2, m3;

	vec3 n01, n12, n23, n30;
	vec3 n0123;
};

patch in pnQuad pn;

out vec3 gnormal;
out vec3 gpos;
out vec2 gtex_coord;

in float frame_id_te[];
out float frame_id_g;

vec3 interpolate3(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3)
{
	vec3 a = mix(v0, v1, gl_TessCoord.x);
	vec3 b = mix(v3, v2, gl_TessCoord.x);
	return mix(a, b, gl_TessCoord.y);
}

float interpolate1(in float v0, in float v1, in float v2, in float v3)
{
	float a = mix(v0, v1, gl_TessCoord.x);
	float b = mix(v3, v2, gl_TessCoord.x);
	return mix(a, b, gl_TessCoord.y);
}

vec3 pt_pi(vec3 q, vec3 p, vec3 n)
{
	return q-dot(q-p, n)*n;
}

vec3 pt_q(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float u, float v)
{
	return (1.0-u)*(1.0-v)*p0 + u*(1.0-v)*p1 + (1.0-u)*v*p3 + u*v*p2;
}

vec4 cr(float t)
{
	return vec4(2.0*t*t*t-3.0*t*t+1.0, t*t*t-2.0*t*t+t, -2.0*t*t*t + 3.0*t*t, t*t*t-t*t);
}

vec4 Bn(float t)
{
   float a = 1.0-t;
   return vec4(a*a, 2.0*a*t, t*t, 0.0);
}

void main()
{
	gtex_coord = gl_TessCoord.xy;

	float u = gl_TessCoord.y;
	float v = gl_TessCoord.x;

	/*
	vec3 pnPos  = interpolate3(pn.b0, pn.b1, pn.b2, pn.b3);
	vec3 pnNorm = normalize(interpolate3(pn.n0, pn.n1, pn.n2, pn.n3));
	//*/

	//catmull-rom
	//*
	vec3 p0 = mix(pn.b0, pn.b1, v);
	vec3 p1 = mix(pn.b3, pn.b2, v);
	vec3 m0 = mix(pn.m0, pn.m1, v);
	vec3 m1 = mix(pn.m3, pn.m2, v);

	vec4 t = cr(u);

	vec3 pnPos = t[0]*p0 + t[1]*m0 + t[2]*p1 + t[3]*m1;
	//vec3 pnNorm = normalize(interpolate3(pn.n0, pn.n1, pn.n2, pn.n3));

	vec4 Bu = Bn(v);
	vec4 Bv = Bn(u);

	vec3 pnNorm=	Bu[0]*Bv[0]*pn.n0  + Bu[1]*Bv[0]*pn.n01		+ Bu[2]*Bv[0]*pn.n1 +
            Bu[0]*Bv[1]*pn.n30 + Bu[1]*Bv[1]*pn.n0123	+ Bu[2]*Bv[1]*pn.n12 +
            Bu[0]*Bv[2]*pn.n3  + Bu[1]*Bv[2]*pn.n23		+ Bu[2]*Bv[2]*pn.n2;

	pnNorm = normalize(pnNorm);
	//*/

	gpos = pnPos;
	gnormal = pnNorm;

	gl_Position = vec4(pnPos, 1.0);

	frame_id_g = interpolate1(frame_id_te[0], frame_id_te[1], frame_id_te[3], frame_id_te[2]);
	
}

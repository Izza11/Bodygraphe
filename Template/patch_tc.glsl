#version 430 core
layout (vertices = 4) out;

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

in float frame_id_tc[];
out float frame_id_te[];

patch out pnQuad pn;
vec3 N[4];
vec3 P[4];

vec3 calcNormal(vec3 p0, vec3 p1, vec3 p2)
{
	return cross(p1-p0, p2-p0);
}

vec3 calcNormalX(int i, int j, int k)
{
	vec3 n = cross(gl_in[j].gl_Position.xyz-gl_in[i].gl_Position.xyz, gl_in[k].gl_Position.xyz-gl_in[i].gl_Position.xyz);
	//return normalize(n);
	return n;
}


float vij(int i, int j)
{
	vec3 Pj_minus_Pi = P[j]	- P[i];
	vec3 Ni_plus_Nj  = N[i] + N[j];
	return 2.0*dot(Pj_minus_Pi, Ni_plus_Nj)/dot(Pj_minus_Pi, Pj_minus_Pi);
}

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	frame_id_te[gl_InvocationID] = frame_id_tc[gl_InvocationID];

	if(gl_InvocationID == 0)
	{
		//set tessellation levels
		const int utess = 4;
		const int vtess = 4;
		gl_TessLevelOuter[0] = utess;
		gl_TessLevelOuter[1] = vtess;
		gl_TessLevelOuter[2] = utess;
		gl_TessLevelOuter[3] = vtess;

		gl_TessLevelInner[0] = utess;
		gl_TessLevelInner[1] = vtess;
		
		P[0] = gl_in[2].gl_Position.xyz;
		P[1] = gl_in[3].gl_Position.xyz;
		P[2] = gl_in[5].gl_Position.xyz;
		P[3] = gl_in[4].gl_Position.xyz;

		float w = 1.0;
		pn.m0 = w*(P[3]-P[0]);
		pn.m1 = w*(P[2]-P[1]);
		pn.m2 = w*(gl_in[7].gl_Position.xyz - gl_in[5].gl_Position.xyz);
		pn.m3 = w*(gl_in[6].gl_Position.xyz - gl_in[4].gl_Position.xyz);
		
		N[0] = normalize(calcNormalX(2,0,3) + calcNormalX(2,3,4));
		N[1] = normalize(calcNormalX(3,2,1) + calcNormalX(3,5,2));
		N[2] = normalize(calcNormalX(5,4,3) + calcNormalX(5,7,4));
		N[3] = normalize(calcNormalX(4,2,5) + calcNormalX(4,5,6));


		pn.b0 = P[0]; pn.b1 = P[1]; pn.b2 = P[2]; pn.b3 = P[3];
		pn.n0 = N[0]; pn.n1 = N[1]; pn.n2 = N[2]; pn.n3 = N[3];

		pn.n01 = normalize(N[0] + N[1] - vij(0,1)*(P[1]-P[0]));
		pn.n12 = normalize(N[1] + N[2] - vij(1,2)*(P[2]-P[1]));
		pn.n23 = normalize(N[2] + N[3] - vij(2,3)*(P[3]-P[2]));
		pn.n30 = normalize(N[3] + N[0] - vij(3,0)*(P[0]-P[3]));
		pn.n0123 = (2.0*(pn.n01 + pn.n12 + pn.n23 + pn.n30) + pn.n0 + pn.n1 + pn.n2 + pn.n3)/12.0;
	}
}
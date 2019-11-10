#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;


layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform float time = 0.0;
layout(location = 5) uniform float params[10];
layout(location = 19) uniform int displacement_mode = 0;
layout(location = 20) uniform float displacement_amount = 0.0;

in vec3 gpos[];
out vec3 pos;

in vec3 gnormal[];
out vec3 normal;

in vec2 gtex_coord[];
out vec2 tex_coord;

in float frame_id_g[];
out float frame_id;

float rand(vec2 co);

vec4 compute_displacement(int i);

void main() 
{
	
	for(int i=0; i<3; i++)
	{
		vec4 dp = compute_displacement(i);
		gl_Position = PVM*(gl_in[i].gl_Position+dp);
		pos = gpos[i];
		normal = gnormal[i];
		tex_coord = gtex_coord[i];
		frame_id = frame_id_g[i];
		EmitVertex();
	}
	EndPrimitive();

}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 compute_displacement(int i) 
{
	vec4 dp = vec4(0.0);
	if(displacement_amount<=0.0)
	{
		return dp;
	}

	if(displacement_mode == 0)
	{
		dp = displacement_amount*sin(5.0*gtex_coord[i].x+time)*vec4(gnormal[i], 0.0);
		return dp;
	}

	if(displacement_mode == 1)
	{
		//disintegration
		float r1 = displacement_amount*(rand(gpos[0].xy)-0.5);
		float r2 = displacement_amount*(rand(gpos[1].yx)-0.5);
		float ft = displacement_amount*frame_id_g[0];
	
	
		dp.x = r1*ft;
		dp.z = r2*ft;
		dp.y = -ft;

		float w = smoothstep(0.4, 0.8, frame_id_g[0]);

		return w*dp;
	}

	if(displacement_mode == 2)
	{
		dp = displacement_amount*sin(150.0*params[9]*frame_id_g[i]+time)*vec4(gnormal[i], 0.0);
		return dp;
	}

	if(displacement_mode == 3)
	{
		float t = 0.80;
		if(rand(gtex_coord[0]) > t)
		{
			dp.y = smoothstep(0.4, 0.8, frame_id_g[0])*displacement_amount*frame_id_g[0];
		}
	}

	return dp;
}
#version 450 core 
layout (quads, fractional_even_spacing, ccw) in;

layout(location = 1) uniform float time = 0.0;

out vec2 gtex_coord;

void main()
{
	float u  =   gl_TessCoord.x;
	float v  =   gl_TessCoord.y;

	gtex_coord = gl_TessCoord.xy;

	gl_Position = u*v*gl_in[0].gl_Position + (1.0-u)*v*gl_in[1].gl_Position + u*(1.0-v)*gl_in[2].gl_Position + (1.0-u)*(1.0-v)*gl_in[3].gl_Position;
	
}

#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;


layout(location = 0) uniform mat4 PVM;
layout(location = 1) uniform float time = 0.0;

in vec2 gtex_coord[];
out vec2 tex_coord;

void main() 
{
	gl_Position = PVM*gl_in[0].gl_Position;
	tex_coord = gtex_coord[0];
	EmitVertex();

	gl_Position = PVM*gl_in[1].gl_Position;
	tex_coord = gtex_coord[1];
	EmitVertex();

	gl_Position = PVM*gl_in[2].gl_Position;
	tex_coord = gtex_coord[2];
	EmitVertex();

	EndPrimitive();

}
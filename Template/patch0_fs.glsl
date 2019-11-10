#version 450

out vec4 fragcolor;   

layout(location = 1) uniform float time;

in vec2 tex_coord;

void main(void)
{   
	fragcolor = vec4(tex_coord, 1.0, 1.0);
}


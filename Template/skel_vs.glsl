#version 450        

layout(location = 0) in vec3 pos_attrib;

layout(location = 0) uniform mat4 PVM;



void main(void)
{
	gl_Position = PVM*vec4(pos_attrib, 1.0);
}
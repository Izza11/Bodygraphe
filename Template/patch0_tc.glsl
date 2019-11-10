#version 450 core
layout (vertices = 4) out;

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	if(gl_InvocationID == 0)
	{
		//set tessellation levels
		gl_TessLevelOuter[0] = 2;
		gl_TessLevelOuter[1] = 6;
		gl_TessLevelOuter[2] = 2;
		gl_TessLevelOuter[3] = 6;

		gl_TessLevelInner[0] = 6;
		gl_TessLevelInner[1] = 2;
	}
}
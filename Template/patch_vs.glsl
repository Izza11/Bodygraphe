#version 450 core

layout(location = 0) in vec3 pos_attrib;

out float frame_id_tc;

const int bones_per_skel = 25;
const float num_frames = 80.0;

void main(void)
{
	gl_Position = vec4(pos_attrib, 1.0);
	frame_id_tc = float(gl_VertexID/bones_per_skel)/num_frames;
}
#version 450

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec2 in_TexCoords;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
}vs_out;

void main()
{
	vs_out.TexCoord = in_TexCoords;
	gl_Position = vec4(vec3(in_Pos.xy, 0), 1.0);
}
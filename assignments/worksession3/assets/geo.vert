#version 450
layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
}vs_out;

uniform mat4 _Model;
uniform mat4 _ViewProjection;

void main()
{
	vs_out.WorldPos = vec3(_Model * vec4(in_Pos,1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * in_Normal;

	vs_out.TexCoord = in_TexCoords;

	gl_Position = _ViewProjection * _Model * vec4(in_Pos, 1.0);
}
#version 450

layout(location = 0) in vec3 in_Pos;

uniform mat4 _Model;
uniform mat4 _ViewProjection;

out vec3 WorldPos;

void main()
{
	gl_Position = _ViewProjection * _Model * vec4(in_Pos, 1.0);
}
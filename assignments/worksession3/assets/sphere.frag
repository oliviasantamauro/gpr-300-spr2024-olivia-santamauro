#version 450

out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 LightColor;

void main()
{
	FragColor = vec4(LightColor, 1.0);
}
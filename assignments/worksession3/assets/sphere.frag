#version 450

out vec4 FragColor;

uniform vec4 _LightColor;

void main()
{
	FragColor = vec4(_LightColor);
}
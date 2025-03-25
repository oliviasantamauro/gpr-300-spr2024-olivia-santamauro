#version 450

out vec4 FragColor;

uniform vec4 LightColor;

void main()
{
	FragColor = vec4(LightColor);
}
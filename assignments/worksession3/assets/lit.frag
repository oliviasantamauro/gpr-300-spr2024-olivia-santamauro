#version 450

layout(location = 0)out vec4 FragAlbedo;
layout(location = 1)out vec4 FragPos;
layout(location = 2)out vec4 FragNormal;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
}fs_in;

uniform vec3 _EyePos;
uniform sampler2D _MainTex;

void main()
{
	vec3 color = texture(_MainTex,fs_in.TexCoord).rgb;
	FragAlbedo = vec4(color, 1.0);
	FragPos = vec4(fs_in.WorldPos, 1.0);
	FragNormal = vec4(fs_in.WorldNormal.xyz, 1.0);
}
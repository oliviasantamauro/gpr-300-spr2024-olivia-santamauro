#version 450

//vertex attributes
layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;
layout(location = 3) in vec3 in_Tangent;

uniform mat4 _Model;
uniform mat4 _VeiwProjection;
uniform mat4 _LightSpaceMatrix;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
	vec4 fragPosLightSpace;
}vs_out;


void main()
{
	//model to world space converstions
	vs_out.WorldPos = vec3(_Model * vec4(in_Pos,1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * in_Normal;

	vec3 T = normalize(vec3(_Model * vec4(in_Tangent, 0.0)));
	vec3 N = normalize(vec3(_Model * vec4(in_Normal, 0.0)));

	T = normalize(T - dot(T,N) * N);

	vec3 B = cross(N,T);

	vs_out.TBN = transpose(mat3(T,B,N));

	//texture
	vs_out.TexCoord = in_TexCoords;

	vs_out.fragPosLightSpace = _LightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	gl_Position = _VeiwProjection * _Model * vec4(in_Pos, 1.0);
}
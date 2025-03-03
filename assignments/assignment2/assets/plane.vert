#version 450

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoords;
layout(location = 3) in vec3 in_Tangent;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
	vec4 fragPosLightSpace;
}vs_out;

uniform mat4 model;
uniform mat4 viewProjection;
uniform mat4 _LightSpaceMatrix;

void main(){

	vs_out.WorldPos = vec3(model * vec4(in_Pos,1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(model))) * in_Normal;
	vs_out.TexCoord = in_TexCoords;
	vs_out.fragPosLightSpace = _LightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	gl_Position = viewProjection * model * vec4(in_Pos,1.0);
}

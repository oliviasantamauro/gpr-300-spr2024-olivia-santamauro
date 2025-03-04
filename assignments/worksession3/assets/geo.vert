#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out Surface {
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_surface;

uniform mat4 model;
uniform mat4 viewProjection;

void main(){

	vs_surface.WorldPos = vec3(model * vec4(vPos,1.0));
	vs_surface.WorldNormal = transpose(inverse(mat3(model))) * vNormal;
	vs_surface.TexCoord = vTexCoord;

	gl_Position = viewProjection * model * vec4(vPos,1.0);
}

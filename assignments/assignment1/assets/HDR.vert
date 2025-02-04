#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexCoord;

out vec2 vs_texcoord;

void main(){

	vs_texcoord = vTexCoord;
	gl_Position =  vec4(vPos.xy, 0.0, 1.0);
}
#version 450

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;
layout(location = 2) out vec4 FragColor2;

in Surface {
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

void main(){

	vec3 obj_color = fs_in.WorldNormal * 0.5 + 0.5;
	FragColor0 = vec4(obj_color, 1.0);
	FragColor1 = vec4(fs_in.WorldPos.xyz, 1.0);
	FragColor2 = vec4(fs_in.WorldNormal.xyz, 1.0);

}
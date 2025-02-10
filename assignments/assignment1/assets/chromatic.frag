#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float strength;

const vec3 offset = vec3(0.009, 0.006, -0.006);
const vec2 direction = vec2(1.0);

void main(){
	vec3 scaledOffset = offset * strength;
    FragColor.r = texture(texture0, vs_texcoord + (direction + vec2(scaledOffset.r))).r;
    FragColor.g = texture(texture0, vs_texcoord + (direction + vec2(scaledOffset.g))).g;
    FragColor.b = texture(texture0, vs_texcoord + (direction + vec2(scaledOffset.b))).b;
}

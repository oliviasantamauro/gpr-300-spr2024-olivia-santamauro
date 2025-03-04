#version 450

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D g_albedo;
uniform sampler2D g_position;
uniform sampler2D g_normal;

void main() {
	vec3 albedo = texture(g_albedo, vs_texcoord).rgb;
	vec3 position = texture(g_position, vs_texcoord).rgb;
	vec3 normal = texture(g_normal, vs_texcoord).rgb;
	FragColor = vec4(albedo, 1.0);
}
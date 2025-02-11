#version 450

out vec4 FragColor;

in vec3 to_camera;
in vec2 vs_texcoords;

uniform vec3 water_color;
uniform sampler2D water_tex;
uniform sampler2D water_spec;
uniform sampler2D water_warp;
uniform float tiling;
uniform float time;
uniform float warp_strength;
uniform float b2;
uniform float spec_scale;

const vec3 reflect_color = vec3(1.0, 0.0, 0.0);
const float brightness_lower = 0.2f;
const float brightness_upper = 0.9f;

void main(){

	vec2 uv = vs_texcoords;

	//warp
	vec2 warp_uv = vs_texcoords * tiling;
	vec2 warp_scroll = vec2(0.5, 0.5) * time;
	vec2 warp = texture(water_warp, warp_uv + warp_scroll).xy * warp_strength;
	warp = (warp * 2.0) - 1.0;

	//albedo
	vec2 albedo_uv = vs_texcoords * tiling;
	vec2 albedo_scroll = vec2(-0.5, 0.5) * time;
	vec4 albedo = texture (water_tex, albedo_uv + warp + albedo_scroll);

	//specular
	vec2 spec_uv = vs_texcoords * spec_scale;
	vec3 smp1 = texture(water_spec, spec_uv + vec2(1.0, 0.0) * time).rgb;
	vec3 smp2 = texture(water_spec, spec_uv + vec2(1.0, 1.0) * time).rgb;
	vec3 spec = smp1 + smp2;

	float brightness = dot(spec, vec3(0.299, 0.587, 0.114));
	if (brightness <= brightness_lower || brightness >= brightness_upper)
	{
		discard;
	}

	float fresnel = dot(normalize(to_camera), vec3(0.0, 1.0, 0.0));
	vec3 finalColor = mix(spec, water_color + vec3(albedo.a), fresnel);

	FragColor = vec4(finalColor, 1.0);
	
}

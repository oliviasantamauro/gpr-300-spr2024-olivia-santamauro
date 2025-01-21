#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos;
	vec3 WorldNormal;

	vec2 TexCoord;
}fs_surface;

uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //ambient
	float Kd; //diffuse
	float Ks; //specular
	float Shininess;
};

struct Palette{
	vec3 highlight;
	vec3 shadow;
};

uniform Palette _Palette;
uniform Material _Material;
uniform sampler2D albedo;
uniform sampler2D zatoon;


vec3 toon_lighting(vec3 normal, vec3 light_dir) {
	float diff = (dot(normal, light_dir) + 1.0) * 0.5;
	float step = texture(zatoon, vec2(diff)).r;

	vec3 light_color = mix(_Palette.shadow, _Palette.highlight, step);

	return light_color;

}

void main(){

	vec3 normal = normalize(fs_surface.WorldNormal);

	vec3 toLight = -_LightDirection;

	//float diffuseFactor = max(dot(normal,toLight),0.0);
	//vec3 toEye = normalize(_EyePos - fs_surface.WorldPos);
	//vec3 h = normalize(toLight + toEye);
	//float specularFactor = pow(max(dot(normal,h),0.0),128);

	vec3 lightColor = toon_lighting(normal, toLight);
	//lightColor += _AmbientColor * _Material.Ka;
	vec3 objectColor = texture(albedo,fs_surface.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor,1.0);

}

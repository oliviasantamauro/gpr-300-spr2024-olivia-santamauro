#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos;
	vec3 WorldNormal;

	vec2 TexCoord;
}fs_surface;

uniform sampler2D _MainTex;
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
uniform Material _Material;


void main(){

	vec3 normal = normalize(fs_surface.WorldNormal);

	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight),0.0);

	vec3 toEye = normalize(_EyePos - fs_surface.WorldPos);
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),128);

	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor+=_AmbientColor * _Material.Ka;
	vec3 objectColor = texture(_MainTex,fs_surface.TexCoord).rgb;
	FragColor = vec4(objectColor * lightColor,1.0);

}

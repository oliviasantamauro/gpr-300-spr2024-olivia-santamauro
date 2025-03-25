#version 450

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
}fs_in;

struct Material{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
	
};
uniform Material _Material;
uniform sampler2D _Coords;
uniform sampler2D _Normals;
uniform sampler2D _Albedo;

uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _EyePos;

out vec4 FragColor;
vec3 LightDirection = vec3(0.0,-1.0, 0.0);

struct PointLight{
	vec3 position;
	float radius;
	vec4 color;
};
#define MAX_POINT_LIGHTS 64
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

float attenuateExponential(float distance, float radius){
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
	
}


vec3 calcPointLight(PointLight light,vec3 normal,vec3 pos){
	vec3 diff = light.position - pos;

	//Direction toward light position
	vec3 toLight = normalize(diff);

	float diffuseFactor = max(dot(normal,toLight),0.0);
	vec3 diffuseColor = _LightColor * diffuseFactor;

	vec3 toEye = normalize(_EyePos - pos);
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);

	vec3 lightColor = (diffuseFactor + specularFactor) * light.color.rgb;

	//Attenuation
	float d = length(diff); //Distance to light
	lightColor *= attenuateExponential(d,light.radius);
	return lightColor;
}


vec3 blinnPhong(vec3 WorldNormal, vec3 WorldPos, vec3 _lightColor, vec3 LightPos)
{
	vec3 normal = normalize(WorldNormal);
	vec3 toLight = normalize(LightPos - WorldPos);

	float diffuseFactor = max(dot(normal,toLight),0.0);
	vec3 diffuseColor = _LightColor * diffuseFactor;

	vec3 toEye = normalize(_EyePos - WorldPos);
	vec3 h = normalize(toLight + toEye);

	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);

	vec3 lightColor = (diffuseColor * _Material.Kd + specularFactor * _Material.Ks) * _lightColor;

	return lightColor;

}
void main()	
{

	vec3 lightColor = texture(_Albedo, fs_in.TexCoord).rgb;
	vec3 lightPos = vec3(0.0, 5.0, 0.0);

	vec3 normal = texture(_Normals, fs_in.TexCoord).xyz;
	vec3 position = texture(_Coords, fs_in.TexCoord).xyz;
	vec3 totalLight = vec3(0);

	totalLight += blinnPhong(normal, position, lightColor, lightPos);
	for(int i=0; i<MAX_POINT_LIGHTS; i++) {
		totalLight += calcPointLight(_PointLights[i], normal, position);
	}

	vec3 albedo = texture(_Albedo,fs_in.TexCoord).rgb;
	FragColor = vec4(albedo * totalLight,0);

}
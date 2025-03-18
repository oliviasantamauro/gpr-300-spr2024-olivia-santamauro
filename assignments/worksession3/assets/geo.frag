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

	vec3 color = blinnPhong(normal, position, lightColor, lightPos);
	FragColor = vec4(color, 1.0);

}
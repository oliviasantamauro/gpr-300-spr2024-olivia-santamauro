#version 450

out vec4 FragColor;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	mat3 TBN;
	vec4 fragPosLightSpace;

}fs_in;

struct Material{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
	
};

//uniforms
uniform Material _Material;

uniform sampler2D _MainTex;
uniform sampler2D _ShadowMap;

uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);
uniform float _Bias;

uniform vec3 _EyePos;


float shadowCalculation(vec4 fragPosLightSpace, float bias)
{
	vec3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(_ShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 

	if(projCoords.z  <= 0.0 || projCoords.z > 1.0)
	 {
		return 0.0;
	 }

	return shadow;
}

vec3 blinnPhong()
{
	vec3 normal = normalize(fs_in.WorldNormal);
	normal = normalize(normal * 2.0 - 1.0);

	vec3 toLight = _LightDirection;
	toLight = -toLight;

	//defuse lighting
	float diffuseFactor = max(dot(normal,toLight),0.0);
	vec3 diffuseColor = _LightColor * diffuseFactor;
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);

	//blinnphong
	vec3 h = normalize(toLight + toEye);
	
	//specular
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);

	vec3 lightColor = (_Material.Kd * diffuseColor + _Material.Ks * specularFactor) * _LightColor;
	return lightColor;
}
void main()
{
	
	vec3 lightColor = blinnPhong();
	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;
	float bias = max(0.05 * (1.0 - dot(fs_in.WorldNormal, _LightDirection)), _Bias);
	float shadow = shadowCalculation(fs_in.fragPosLightSpace, bias);
	vec3 finalColor = ((_AmbientColor * _Material.Ka) + (1.0 - shadow) * lightColor ) * objectColor;

	FragColor = vec4(finalColor, 1.0);
}
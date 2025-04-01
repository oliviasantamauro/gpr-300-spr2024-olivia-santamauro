#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_surface;

uniform sampler2D _MainTex;
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

const float PI = 3.14159265359;

uniform float roughness;
uniform float metallic;

//cache dot products
float VdotN = 0.0;
float LdotN = 0.0;
float NdotH = 0.0;
float NdotV = 0.0;
float NdotL = 0.0;

float D() {
	float alpha2 = pow(roughness, 4.0);
	float denom = PI * pow(pow(NdotH, 2.0) * (alpha2 - 1.0) + 1.0, 2.0);
	return (alpha2 / denom);
}

float GeometrySchlickGGX(float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float G(float k)
{
    float ggx1 = GeometrySchlickGGX(k);
    float ggx2 = GeometrySchlickGGX(k);
	
    return ggx1 * ggx2;
}

vec3 CookTorrence(vec3 Fresnel) {

	vec3 num = D() * Fresnel * G(1.0);
	float denom = 4.0 * VdotN * LdotN;
	return num / denom;
}

vec3 BRDF(float cosTheta) {
	
	// fresnel-schlick
	vec3 lambert = vec3(1.0, 0.0, 0.0) / PI;

	vec3 f0 = lambert;
	vec3 fresnel = f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5);

	vec3 Ks = fresnel;
	vec3 Kd = (1 - Ks) * (1.0 - metallic);

	vec3 diffuse = (Kd * lambert);
	vec3 specular = (Ks * CookTorrence(fresnel));

	return diffuse + specular;
}

vec3 outgoing_light(vec3 toEye, vec3 fragPos)
{

	vec3 toLight = -_LightDirection;
	vec3 normal = normalize(fs_surface.WorldNormal);
	float diffuseFactor = max(dot(normal,toLight),0.0);
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),128);
	
	vec3 EMITTED = vec3(0);
	vec3 RADIANCE = vec3(0);

	vec3 in_dir = _LightDirection;
	float cosTheta = dot(toEye, h);
	vec3 brdf = BRDF(cosTheta);
	float NdotL = dot(normal, in_dir);

	RADIANCE += brdf * _LightColor * NdotL;

	return RADIANCE + EMITTED;
}

void main(){

	vec3 toEye = normalize(_EyePos - fs_surface.WorldPos);

	vec3 outgoingColor = outgoing_light(toEye, fs_surface.WorldPos);

	FragColor = vec4(outgoingColor, 1.0);

}

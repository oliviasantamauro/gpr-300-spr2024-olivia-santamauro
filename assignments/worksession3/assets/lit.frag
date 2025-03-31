#version 450

struct Material{
    float Ka;
    float Kd;
    float Ks;
    float Shininess;
};

struct PointLight{
    vec3 position;
    float radius;
    vec4 color;
};

in Surface{
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 TexCoord;
    mat3 TBN;
}fs_in;

uniform Material _Material;
uniform sampler2D _Coords;
uniform sampler2D _Normals;
uniform sampler2D _Albedo;
uniform vec3 _EyePos;
uniform PointLight _PointLights[625];
uniform vec3 LightDirection = vec3(0.0, -1.0, 0.0);

out vec4 FragColor;

float attenuateExponential(float distance, float radius){
    float i = clamp(1.0 - pow(distance/radius, 4.0), 0.0, 1.0);
    return i * i;
}

vec3 blinnPhong(vec3 normal, vec3 pos, vec3 _lightColor, vec3 LightPos){
    vec3 toLight = normalize(LightPos - pos);
    float diffuseFactor = max(dot(normal, toLight), 0.0);
    vec3 toEye = normalize(_EyePos - pos);
    vec3 h = normalize(toLight + toEye);
    float specularFactor = pow(max(dot(normal, h), 0.0), _Material.Shininess);
    vec3 lightColor = (_lightColor * diffuseFactor * _Material.Kd + specularFactor * _Material.Ks) * _lightColor;
    return lightColor;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 pos){
    vec3 diff = light.position - pos;
    vec3 toLight = normalize(diff);
    float diffuseFactor = max(dot(normal, toLight), 0.0);
    vec3 toEye = normalize(_EyePos - pos);
    vec3 h = normalize(toLight + toEye);
    float specularFactor = pow(max(dot(normal, h), 0.0), _Material.Shininess);
    vec3 lightColor = light.color.rgb * (diffuseFactor * _Material.Kd + specularFactor * _Material.Ks);
    
    float d = length(diff);
    lightColor *= attenuateExponential(d, light.radius);
    
    return lightColor;
}

void main() {
    vec3 pos = texture(_Coords, fs_in.TexCoord).rgb;
    vec3 normal = texture(_Normals, fs_in.TexCoord).rgb;
    vec3 totalLight = vec3(0);

    vec3 directionalLightColor = vec3(1.0);
    totalLight += blinnPhong(normal, pos, directionalLightColor, LightDirection);

    for(int i = 0; i < 64; i++) {
        totalLight += calcPointLight(_PointLights[i], normal, pos);
    }

    vec3 albedo = texture(_Albedo, fs_in.TexCoord).rgb;
    FragColor = vec4(albedo * totalLight, 1.0);
}
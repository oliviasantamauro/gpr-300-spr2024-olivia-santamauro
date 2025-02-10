#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float strength;
uniform vec3 edgeColor;

const float offset = 1.0 / 300.0;
const vec2 offsets[9] = vec2[](
    vec2(-offset, offset),
    vec2(0.0, offset),
    vec2(offset, offset),

    vec2(-offset, 0.0),
    vec2(0.0, 0.0),
    vec2(offset, 0.0),

    vec2(-offset, -offset),
    vec2(0.0, -offset),
    vec2(offset, -offset)
);

const float kernelX[9] = float[](
    -1.0, 0.0, 1.0,
    -2.0, 0.0, 2.0,
    -1.0, 0.0, 1.0
);

const float kernelY[9] = float[](
    -1.0, -2.0, -1.0,
    0.0, 0.0, 0.0,
    1.0, 2.0, 1.0
);

void main(){
    float edgeX = 0.0;
    float edgeY = 0.0;

    for(int i = 0; i < 9; i++)
    {
        vec3 local = texture(texture0, vs_texcoord.xy + offsets[i]).rgb;
        edgeX += dot(local, vec3(0.2126, 0.7152, 0.0722)) * kernelX[i];
        edgeY += dot(local, vec3(0.2126, 0.7152, 0.0722)) * kernelY[i];
    }

    float edge = sqrt(edgeX * edgeX + edgeY * edgeY);
    edge = clamp(edge * strength, 0.0, 1.0);

    vec3 albedo = texture(texture0, vs_texcoord).rgb;
    FragColor = vec4(mix(albedo, edgeColor, edge), 1.0);
}
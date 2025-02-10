#version 450

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, vs_texcoord).rgb;      
    vec3 bloomColor = texture(bloomBlur, vs_texcoord).rgb;

    // additive blending
    hdrColor += bloomColor;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // gamma correction     
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}
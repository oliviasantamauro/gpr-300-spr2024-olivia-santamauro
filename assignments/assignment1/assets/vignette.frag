#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float falloff;
uniform float amount;

void main(){
    vec4 color = texture(texture0, vs_texcoord);
    
    float dist = distance(vs_texcoord, vec2(0.5, 0.5));
    color.rgb *= smoothstep(0.8, falloff * 0.799, dist * (amount + falloff));
    
    FragColor = color;
}
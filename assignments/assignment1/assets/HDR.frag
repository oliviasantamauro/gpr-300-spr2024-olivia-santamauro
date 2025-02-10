#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float exposure;

void main(){

    const float gamma = 2.2;
	vec3 hdr = texture(texture0, vs_texcoord).rgb;
  
    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdr * exposure);
    
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}

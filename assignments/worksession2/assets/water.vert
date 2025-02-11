#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 viewProjection;
uniform vec3 camera_pos;
uniform float time;
uniform float uniform_strength;

out vec2 vs_texcoords;
out vec3 to_camera;

float calculateSurface (float x, float z)
{
	float scale = 10.0;
  float y = 0.0;
  y += (sin(x * 1.0 / scale + time * 1.0) + sin(x * 2.3 / scale + time * 1.5) + sin(x * 3.3 / scale + time * 0.4)) / 3.0;
  y += (sin(z * 0.2 / scale + time * 1.8) + sin(z * 1.8 / scale + time * 1.8) + sin(z * 2.8 / scale + time * 0.8)) / 3.0;
  return y;
}

void main(){

	vec3 pos = vPos;
	pos += calculateSurface(pos.x, pos.z) * uniform_strength;
	pos += calculateSurface(0.0, 0.0) * uniform_strength;
	vec4 world_position = model * vec4(pos, 1.0);
	to_camera = camera_pos - world_position.xyz;
	vs_texcoords = vTexCoord;
	gl_Position = viewProjection * world_position;
}

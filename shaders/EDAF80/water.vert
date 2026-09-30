#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
/* Add time uniform */
uniform float time;
uniform float amplitude;
uniform float frequency;
uniform float phase;
uniform float sharpness;
uniform vec2 direction;



out VS_OUT {
vec3 vertex;
vec3 normal;

} vs_out;
float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
{
	return amplitude * pow(sin(dot(position, direction) * frequency + phase * time) * 0.5 + 0.5, sharpness);
}
float wave_diriv(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time,float diridir)
{
	float theta =
        dot(position, direction) * frequency
        + phase * time;
	return 0.5f* sharpness* frequency* amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness-1)
			+ cos(theta)*diridir;
}
void main()
{
	vec3 displaced_vertex = vertex;
	float x = wave(vertex.xz, direction, amplitude, frequency, phase, sharpness, time);

	displaced_vertex.y += wave(vertex.xz, vec2(-1.0, 0.0), amplitude, frequency, phase, sharpness, time);

	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));
	vs_out.normal = vec3(normal_model_to_world *
	vec4(-wave_diriv(vertex.xz, direction, amplitude, frequency, phase, sharpness, time, direction.x)
	,1
	,-wave_diriv(vertex.xz, direction, amplitude, frequency, phase, sharpness, time, direction.y)
	,0.0));

	gl_Position = vertex_world_to_clip * vec4(vs_out.vertex, 1.0);
}

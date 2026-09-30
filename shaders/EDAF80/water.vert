#version 410

layout (location = 0) in vec3 vertex;

layout (location = 2) in vec2 texcoord;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
/* Add time uniform */
uniform float time;


out VS_OUT {
vec3 vertex;
vec3 tangent;
vec3 binormal;
vec3 normal;
vec2 texcoord;
} vs_out;

float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time, out float dx, out float dz)
{
	float theta =
        dot(position, direction) * frequency + phase * time;

	dx = 0.5f* sharpness* frequency* amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness-1) * cos(theta)*direction.x;
	dz = 0.5f* sharpness* frequency* amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness-1) * cos(theta)*direction.y;

	return amplitude * pow(sin(theta) * 0.5 + 0.5, sharpness);
}



void main()
{
	vec3 displaced_vertex = vertex;

	float dx, dz, dx2, dz2;
	displaced_vertex.y +=  wave(vertex.xz, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, time, dx, dz);
	displaced_vertex.y +=  wave(vertex.xz, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, time, dx2, dz2);

	float dhx = dx + dx2;
	float dhz = dz + dz2;


	vec3 N = normalize(vec3(-dhx, 1.0, -dhz));
	vec3 T = normalize(vec3(1.0, dhx, 0.0));
	vec3 B = normalize(vec3(0.0, dhz, 1.0));


	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));
	vs_out.normal = normalize(vec3(normal_model_to_world * vec4(N, 0.0)));
	vs_out.tangent = vec3(vertex_model_to_world * vec4(T,0.0));
	vs_out.binormal = vec3(vertex_model_to_world * vec4(B,0.0));

	vs_out.texcoord = texcoord;
	//vs_out.TBN = mat3(normal_model_to_world * vec4(T, 0.0), normal_model_to_world * vec4(B, 0.0), normal_model_to_world * vec4(N, 0.0));

	gl_Position = vertex_world_to_clip * vec4(vs_out.vertex, 1.0);
}

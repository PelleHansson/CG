#version 410

uniform vec3 camera_position;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
} fs_in;

out vec4 frag_color;

void main()
{

	vec4 deep_c = vec4(0.0, 0.5, 0.5, 1.0);
	vec4 shallow_c = vec4(0.0, 0.0, 0.1, 1.0);

	vec3 V = normalize(camera_position - fs_in.vertex);
    vec3 n = normalize(fs_in.normal);

	float facing = 1.0 - max(dot(V, n), 0.0);

	frag_color = mix(deep_c, shallow_c,	facing);
}

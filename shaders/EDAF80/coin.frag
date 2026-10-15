#version 410

uniform vec3 camera_position;   
uniform vec3 coin_color;        

in VS_OUT {
	vec3 vertex;
	vec3 normal;
} fs_in;

out vec4 frag_color;         

void main()
{
	vec3 n = normalize(fs_in.normal);
	vec3 l = normalize(camera_position - fs_in.vertex);   // 카메라에서 비추는 빛의 방향 
	float diff = max(dot(n, l), 0.0);
	frag_color = vec4( coin_color * (0.3 + 0.7 * diff), 1.0);
}

#version 410

layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform float elapsed_time_s;

out VS_OUT {
	vec3 vertex;
	vec2 texcoord;
	vec3 normal;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
	vec3 tangent;
	vec3 binormal;
} vs_out;

float wave(vec2 position, vec2 direction, float amplitude,
           float frequency, float phase, float sharpness, float time,
		   out float dx, out float dz)
{
	float sth= dot(position, direction)*frequency + time * phase;
	float alpha = sin(sth)*0.5 +0.5;

	float sth2 = 0.5*sharpness*frequency*amplitude*pow(alpha, sharpness-1.0)*cos(sth);
	dx = sth2 * direction.x;
	dz = sth2 * direction.y; //direction의 두 번째 칸 = z

	return amplitude * pow (alpha, sharpness) ;
}

void main()
{
	// model 좌표에서 파도 적용
	vec3 displaced_vertex = vertex;

	float dx1, dz1, dx2, dz2;
	float w1 = wave(vertex.xz, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, elapsed_time_s, dx1, dz1);
	float w2 = wave(vertex.xz, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, elapsed_time_s, dx2, dz2);
	displaced_vertex.y += w1 + w2;

	// normal도 model 좌표에서 계산
	float dHdx = dx1 + dx2;
	float dHdz = dz1 + dz2;
	vec3 n = vec3(-dHdx, 1.0, -dHdz);

	vec3 t = vec3(1.0, dHdx, 0.0);
	vec3 b = vec3(0.0, dHdz, 1.0);

	// world 좌표로 변환
	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));
	vs_out.texcoord = texcoord;
	vs_out.normal = vec3(normal_model_to_world * vec4(n, 0.0));

	vs_out.tangent = vec3(vertex_model_to_world * vec4(t,0.0));
	vs_out.binormal = vec3(vertex_model_to_world * vec4(b,0.0));
	gl_Position = vertex_world_to_clip * vertex_model_to_world *vec4(displaced_vertex, 1.0);

	vec2 texScale = vec2(8, 4);
	float normalTime = mod(elapsed_time_s, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0.0);
	vs_out.normalCoord0.xy = texcoord.xy * texScale + normalTime * normalSpeed;
	vs_out.normalCoord1.xy = texcoord.xy * texScale * 2 + normalTime * normalSpeed * 4;
    vs_out.normalCoord2.xy = texcoord.xy * texScale * 4 + normalTime * normalSpeed * 8;
}

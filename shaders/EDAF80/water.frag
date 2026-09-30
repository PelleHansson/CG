#version 410

uniform float elapsed_time_s;
uniform vec3 camera_position;
uniform samplerCube SkyboxTexture;
uniform sampler2D WavesTexture;

in VS_OUT {
	vec3 vertex;
	vec2 texcoord;
	vec3 normal;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
	vec3 tangent;
	vec3 binormal;
} fs_in;

out vec4 frag_color;

void main()
{
	// V : 점 -> 카메라 방향
	vec3 V = normalize(camera_position - fs_in.vertex);

	// normal map을 크기/속도가 다른 3개 위치에서 읽고, 색(0~1) -> 방향(-1~1)으로 변환 
	vec3 n0 = texture(WavesTexture, fs_in.normalCoord0).xyz * 2.0 - 1.0;
	vec3 n1 = texture(WavesTexture, fs_in.normalCoord1).xyz * 2.0 - 1.0;
	vec3 n2 = texture(WavesTexture, fs_in.normalCoord2).xyz * 2.0 - 1.0;

	// normal map에서 읽은 화살표 n_bump으로 합치기 
	vec3 n_bump = normalize(n0 + n1 + n2); 
	mat3 TBN = mat3(normalize(fs_in.tangent), normalize(fs_in.binormal), normalize(fs_in.normal));
	// 잔물결까지 반영된 최종 normal
	vec3 n = normalize(TBN * n_bump); 

	// 기본 물 색
	float facing = 1.0 - max(dot(V, n), 0.0);
	vec4 deep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 shallow = vec4(0.0, 0.5, 0.5, 1.0);

	// 반사
	vec3 R = reflect(-V, n); // 반사 방향
	vec4 reflection = texture(SkyboxTexture, R);
	vec4 water_color = mix(deep, shallow, facing);

	// Fresnel
	float R0 = 0.02037; // 공기->물 반사율
	float fresnel = R0 + (1.0 - R0) * pow((1.0 - max( dot(V, n), 0)), 5.0);

	// 굴절
	vec3 T = refract(-V, n, 1.0 / 1.33); // 굴절 방향
	vec4 refraction = texture(SkyboxTexture, T);

	// frag_color = water_color + reflection * fresnel;
	frag_color = water_color + reflection * fresnel + refraction * (1.0 - fresnel);
}

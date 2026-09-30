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
	vec3 V = normalize(camera_position - fs_in.vertex);

	vec3 n0 = texture(WavesTexture, fs_in.normalCoord0).xyz * 2.0 - 1.0;
	vec3 n1 = texture(WavesTexture, fs_in.normalCoord1).xyz * 2.0 - 1.0;
	vec3 n2 = texture(WavesTexture, fs_in.normalCoord2).xyz * 2.0 - 1.0;

	vec3 n_bump = normalize(n0 + n1 + n2); // normal map에서 읽은 화살표 n_bump
	mat3 TBN = mat3(normalize(fs_in.tangent), normalize(fs_in.binormal), normalize(fs_in.normal));
	vec3 n = normalize(TBN * n_bump); // 잔물결까지 반영된 최종 normal

	float facing = 1.0 - max(dot(V, normalize(n)), 0.0);
	vec4 deep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 shallow = vec4(0.0, 0.5, 0.5, 1.0);
	vec3 R = reflect(-V, n); // 반사 방향
	vec4 reflection = texture(SkyboxTexture, R);
	vec4 water_color = mix(deep, shallow, facing);

	float R0 = 0.02037; // 물 반사율 
	float fresnel = R0 + (1.0 - R0) * pow((1.0 - dot(V, n)), 5.0);

	vec3 T = refract(-V, n, 1.0 / 1.33); // 굴절 방향
	vec4 refraction = texture(SkyboxTexture, T);

	// frag_color = water_color + reflection * fresnel;
	frag_color = water_color + reflection * fresnel + refraction * (1.0 - fresnel);
}

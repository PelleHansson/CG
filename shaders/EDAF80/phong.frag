#version 410

uniform vec3 light_position;
uniform int use_normal_mapping;

uniform vec3 ambient_colour; // Material ambient 
uniform vec3 diffuse_colour; // Material diffuse
uniform vec3 specular_colour; // Material specular
uniform float shininess_value;

uniform sampler2D DiffuseTexture; 
uniform sampler2D SpecularTexture;
uniform sampler2D NormalTexture;

in VS_OUT { // from vertex shader
	vec3 fN;
	vec3 fL;
	vec3 fV;
	vec2 texCoord;
	vec3 fT;
	vec3 fB;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 N = normalize(fs_in.fN);

	if(use_normal_mapping == 1){
		vec3 normal = texture(NormalTexture, fs_in.texCoord).xyz;
		normal = normal * 2.0 - 1.0; // -1 ~ 1
		vec3 T = normalize(fs_in.fT);
		vec3 B = normalize(fs_in.fB);
		N = normalize(normal.x * T + normal.y * B + normal.z * N);
	}
	vec3 L = normalize(fs_in.fL);
	vec3 V = normalize(fs_in.fV);
	vec3 R = normalize(reflect(-L, N));
	vec3 diffuse = texture(DiffuseTexture, fs_in.texCoord).xyz * max(dot(N, L), 0.0);
	vec3 specular = texture(SpecularTexture, fs_in.texCoord).xyz * pow(max(dot(R, V), 0.0), shininess_value);

	frag_color = vec4(ambient_colour + diffuse + specular, 1.0);
	// frag_color = vec4(texture(DiffuseTexture, fs_in.texCoord).rgb, 1.0); // 텍스쳐 확인
	// frag_color = vec4(vec3(max(dot(N, L), 0.0)), 1.0); // 빛 계산 확인
}

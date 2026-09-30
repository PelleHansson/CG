#version 410

uniform vec3 camera_position;
uniform vec3 light_position;
uniform samplerCube reflection_texture;
uniform sampler2D normal_texture;
uniform int has_normal_texture;
uniform float time;
uniform float water_height;

in VS_OUT {
	vec3 vertex;
	vec3 tangent;
	vec3 binormal;
	vec3 normal;
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	vec4 deep_c = vec4(0.0, 0.5, 0.5, 1.0);
	vec4 shallow_c = vec4(0.0, 0.0, 0.1, 1.0);

	vec3 V = normalize(camera_position - fs_in.vertex);
	vec3 N = normalize(fs_in.normal);

	vec2 texScale = vec2(8, 4);
	float normalTime = mod(time, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0.0);

	vec3 n0 =  texture(normal_texture, (fs_in.texcoord.xy * texScale     + normalTime * normalSpeed)).rgb * 2.0 - 1.0;
	vec3 n1 =  texture(normal_texture, (fs_in.texcoord.xy * texScale * 2 + normalTime * normalSpeed * 4)).rgb * 2.0 - 1.0;
	vec3 n2 =  texture(normal_texture, (fs_in.texcoord.xy * texScale * 4 + normalTime * normalSpeed * 8)).rgb * 2.0 - 1.0;
	vec3 n_bump = normalize(n0 + n1 + n2);
	mat3 TBN = mat3(normalize(fs_in.tangent), normalize(fs_in.binormal), normalize(fs_in.normal));
	N = normalize(TBN * n_bump);

	float facing = 1.0 - max(dot(V, N), 0.0);
	vec3 R = reflect(-V,N);
	vec4 reflection = texture(reflection_texture, R);
	vec4 water_color =  mix(deep_c, shallow_c,	facing);

	float R0= 0.02037;
	float fresnel = R0 + (1- R0) * pow((1 -max( dot(V,N),0)),5); 

	float eta = 1/1.33;
	if(camera_position.y  < water_height)
		eta = 1.33;
	vec4 refraction = texture(reflection_texture, refract(-V, N, eta));
	//vec3 L = normalize(light_position - fs_in.vertex);
	

	frag_color = water_color + reflection* fresnel + refraction * (1- fresnel);
}

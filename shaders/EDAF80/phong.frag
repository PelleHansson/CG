#version 410

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;

uniform sampler2D diffuse_texture;
uniform int has_diffuse_texture;

uniform sampler2D specular_texture;
uniform int has_specular_texture;

uniform sampler2D normal_texture;
uniform int has_normal_texture;

uniform int use_normal_mapping;
uniform mat4 normal_model_to_world;

uniform vec3 light_position;
uniform vec3 camera_position;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 texcoord;
	mat3 TBN;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 V = normalize(camera_position - fs_in.vertex);
	
	vec3 N = normalize(fs_in.normal);

	

	if(use_normal_mapping != 0 && has_normal_texture != 0) {
    	vec3 normal_map = texture(normal_texture, fs_in.texcoord).rgb * 2.0 - 1.0;
		 N = normalize(fs_in.TBN * normal_map);
	}

	vec3 R = reflect(-L,N);

	vec3 diffuse =  vec3(1.0) * max(dot(N, L), 0.0);
    if (has_diffuse_texture != 0)
        diffuse *= texture(diffuse_texture, fs_in.texcoord).rgb;
   else
   diffuse *= diffuse_colour;
	vec3 ambient = ambient_colour;

	vec3 specular =	 vec3(1.0) * pow(max(dot(R, V), 0.0), shininess_value);

    if (has_specular_texture != 0)
		specular *= texture(specular_texture, fs_in.texcoord).rgb;
	else
	specular *= specular_colour;

	frag_color = vec4(diffuse + ambient + specular, 1.0);


}

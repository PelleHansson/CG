#version 410

uniform samplerCube SkyboxTexture;

in VS_OUT {
	vec3 direction;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = texture(SkyboxTexture, fs_in.direction);
}

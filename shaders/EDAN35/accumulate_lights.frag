#version 410

struct ViewProjTransforms
{
	mat4 view_projection;
	mat4 view_projection_inverse;
};

layout (std140) uniform CameraViewProjTransforms
{
	ViewProjTransforms camera;
};

layout (std140) uniform LightViewProjTransforms
{
	ViewProjTransforms lights[4];
};

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;

void main()
{
	vec2 shadowmap_texel_size = 1.0f / textureSize(shadow_texture, 0);

	vec2 texcoord = gl_FragCoord.xy * inverse_screen_resolution;
	vec3 normal = normalize(texture(normal_texture, texcoord).rgb * 2.0f - 1.0f);
	float depth = texture(depth_texture, texcoord).r;

	vec4 pos = camera.view_projection_inverse * vec4(vec3(texcoord, depth), 1.0f);
	vec3 ssp_pos = pos.xyz/pos.w;
	vec3 L = normalize(light_position - ssp_pos);
	vec3 V = normalize(camera_position - ssp_pos);
	vec3 R = normalize(reflect(-1 * L, normal));

	light_diffuse_contribution  = vec4(light_color * light_intensity * max(dot(normal, L), 0.0f), 1.0f);
	light_specular_contribution = vec4(light_color * light_intensity * pow(max(dot(R, V), 0), 8.0f), 1.0f);
}

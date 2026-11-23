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

	vec2 fragcoord = gl_FragCoord.xy * inverse_screen_resolution;
	vec3 normal = normalize(texture(normal_texture, fragcoord).rgb * 2.0f - 1.0f);
	float depth = texture(depth_texture, fragcoord).r;

	vec4 pos = camera.view_projection_inverse * (vec4(fragcoord, depth, 1.0f) * 2.0f - 1.0f);
	vec3 frag_pos = pos.xyz/pos.w;
	vec3 light_vect = light_position - frag_pos;
	float dist = length(light_vect);
	vec3 L = normalize(light_vect);
	vec3 V = normalize(camera_position - frag_pos);
	vec3 R = normalize(reflect(-1 * L, normal));

	float falloff_dist = 1.0f / pow(dist, 2);
	float light_angular = max(dot(-L, light_direction), 0.0f);
	float light_angular_falloff = cos(light_angle_falloff);
	float angular_falloff = max(light_angular / light_angular_falloff - 1.0f, 0.0f);

	vec3 light = light_color * light_intensity * falloff_dist * angular_falloff;

	light_diffuse_contribution  = vec4(light * max(dot(normal, L), 0.0f), 1.0f);
	light_specular_contribution = vec4(light * pow(max(dot(R, V), 0), 8.0f), 1.0f);
}

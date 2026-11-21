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

	vec2 uv = gl_FragCoord.xy * inverse_screen_resolution;
	float d = texture(depth_texture, uv).r;

	vec4 world = camera.view_projection_inverse * vec4(vec3(uv, d) * 2.0 - 1.0, 1.0);
	vec3 world_pos = world.xyz / world.w;


	vec3 tolight = light_position - world_pos; 
	vec3 l = normalize(tolight);
	float sqrDist = dot(tolight, tolight);
	float falloff = 1.0 / max(sqrDist, 0.0001);

	float angleCen= dot(l, normalize(-light_direction));
	float cos_edge = cos(radians(45.0));                       
	float spot = clamp((angleCen - cos_edge) / (1.0 - cos_edge), 0.0, 1.0);

	vec3 n = normalize(texture(normal_texture, uv).xyz * 2.0 - 1.0);
	float nDot = max(dot(n, l), 0.0);

	vec3 r = reflect(-l,n);
	vec3 v = normalize(camera_position -world_pos);


	vec4 light_clip = lights[light_index].view_projection * vec4(world_pos, 1.0);
	vec3 light_ndc = light_clip.xyz / light_clip.w;
	vec3 shadow_coord = light_ndc * 0.5 + 0.5;     
	float lit = 0.0;
	float bias = 0.0005;
	for (int x = -1 ;x < 2; x ++ ) {
		for (int y = -1; y < 2; y ++ ) {
		    float closest = texture(shadow_texture, shadow_coord.xy + vec2(x, y)* shadowmap_texel_size).r;
			lit += (shadow_coord.z - bias < closest) ? 1.0 : 0.0;
		}
	}
	lit /= 9.0;

	vec3 diffuse = lit*light_color* spot *falloff* light_intensity * nDot  ;
	vec3 specular =	 lit* light_color * spot * falloff * light_intensity * (nDot > 0 ? pow(max(dot(r, v), 0.0), 16.0) : 0.0);

	light_diffuse_contribution  = vec4 (diffuse, 1.0) ;
	light_specular_contribution = vec4(specular, 1.0);
}

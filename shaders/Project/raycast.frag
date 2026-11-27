
#version 410

in VS_OUT {
    vec2 texcoord;
} fs_in;

out vec4 frag_color;


uniform mat4 inverseViewProjection;
uniform vec3 cameraPosition;

//uniform vec3 planePosition;
//uniform vec3 planeRight;
//uniform vec3 planeUp;
//uniform vec3 planeNormal;

uniform float sheetWidth;
uniform float sheetHeight;

uniform sampler2D inputArray;


vec3 computeRay(vec2 ndc)
{
	vec4 near = inverseViewProjection * vec4(ndc, -1.0, 1.0);
	vec4 far  = inverseViewProjection * vec4(ndc,  1.0, 1.0);
	near /= near.w;
	far  /= far.w;
	return normalize(far.xyz - near.xyz);
}
bool intersect(vec3 ray, out vec3 hitPosition, out float t) {
	vec3 planePosition = vec3(0.0f, 0.0f, 0.0f);
    vec3 planeNormal = vec3(0,0,1);


	float denom = dot(ray, planeNormal);

	 if (abs(denom) < 0.0001)
        return false;

	t = dot(planePosition - cameraPosition, planeNormal) / denom;

	if(t < 0)
		return false;

	 hitPosition = cameraPosition + t * ray;

	 return true;
}

void main()
{
	 
    vec3 planeRight = vec3(1,0,0);
    vec3 planeUp = vec3(0,1,0);
	float sheetWidth = 10.0f;
	float sheetHeight = 10.0f;
	
    vec2 ndc = fs_in.texcoord * 2.0 - 1.0;

    vec4 farPoint = inverseViewProjection * vec4(ndc, 1.0, 1.0);
    farPoint /= farPoint.w;

    vec3 rayOrigin = cameraPosition;
    vec3 ray = computeRay(ndc);

    

   

    vec3 q; //positionen
    float t; // distansen


   vec3 planePosition = vec3(0.0f, 0.0f, 0.0f);
    if(!intersect(ray ,q, t))
		discard;
   
    vec2 uv = vec2(
        dot(q, planeRight) / sheetWidth + 0.5,
        dot(q, planeUp) / sheetHeight + 0.5
    );

    if (uv.x < 0.0 || uv.x > 1.0 ||
        uv.y < 0.0 || uv.y > 1.0)
        discard;

    frag_color = texture(inputArray, uv);
}

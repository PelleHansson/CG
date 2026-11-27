
#version 410

in VS_OUT {
    vec2 texcoord;
} fs_in;

out vec4 result;

uniform mat4 inverseViewProjection;
uniform vec3 cameraPosition;

//uniform vec3 planePosition;
//uniform vec3 planeRight;
//uniform vec3 planeUp;
//uniform vec3 planeNormal;

uniform float sheetWidth;
uniform float sheetHeight;

uniform sampler2D inputArray;

void main()
{
	 vec3 planePosition = vec3(0.0f, 0.0f, 0.0f);

     vec3 planeNormal = vec3(0,0,1);



    vec3 planeRight = vec3(1,0,0);

    vec3 planeUp = vec3(0,1,0);

	float sheetWidth = 10.0f;
	float sheetHeight = 10.0f;
    vec2 ndc = fs_in.texcoord * 2.0 - 1.0;

    vec4 farPoint = inverseViewProjection * vec4(ndc, 1.0, 1.0);
    farPoint /= farPoint.w;

    vec3 rayOrigin = cameraPosition;
    vec3 rayDirection = normalize(farPoint.xyz - rayOrigin);

    float denominator = dot(rayDirection, planeNormal);

    if (abs(denominator) < 0.0001)
        discard;

    float t = dot(planePosition - rayOrigin, planeNormal) / denominator;

    if (t < 0.0)
        discard;


    vec3 hitPosition = rayOrigin + t * rayDirection;
    vec3 relativePosition = hitPosition - planePosition;

    vec2 uv = vec2(
        dot(relativePosition, planeRight) / sheetWidth + 0.5,
        dot(relativePosition, planeUp) / sheetHeight + 0.5
    );

    if (uv.x < 0.0 || uv.x > 1.0 ||
        uv.y < 0.0 || uv.y > 1.0)
        discard;

    result = texture(inputArray, uv);
}

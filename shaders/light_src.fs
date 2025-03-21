precision highp float;

uniform vec3 lightSrcColor;

void main()
{
    vec3 color = lightSrcColor;
    OUTPUT_COLOR(vec4(color, 1.0));
}

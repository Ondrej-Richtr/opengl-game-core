precision highp float;

OUT_ATTR vec4 FragColor;

uniform vec3 lightSrcColor;

void main()
{
    vec3 color = lightSrcColor;
    FragColor = vec4(color, 1.0);
}

#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

uniform vec3 lightSrcColor;

void main()
{
    vec3 color = lightSrcColor;
    OUTPUT_COLOR(vec4(color, 1.0));
}

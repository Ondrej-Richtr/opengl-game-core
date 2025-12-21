#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

IN_ATTR vec3 TexCoord;

uniform samplerCube skybox;

void main()
{
    vec4 result = TEXTURECUBE(skybox, TexCoord);
    OUTPUT_COLOR(result);
}

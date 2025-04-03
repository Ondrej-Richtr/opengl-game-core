#ifdef GL_ES
    precision mediump float;
#endif

IN_ATTR vec2 TexCoord;
IN_ATTR vec4 Color;

uniform sampler2D inputTexture;

void main()
{
    vec4 sampled = TEXTURE2D(inputTexture, TexCoord);
    OUTPUT_COLOR(Color * sampled);
}

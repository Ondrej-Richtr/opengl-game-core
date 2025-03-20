precision mediump float;

IN_ATTR vec2 TexCoord;
IN_ATTR vec4 Color;

uniform sampler2D inputTexture;

void main()
{
    vec4 sampled = texture(inputTexture, TexCoord);
    OUTPUT_COLOR(Color * sampled);
}

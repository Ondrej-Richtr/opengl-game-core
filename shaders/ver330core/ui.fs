precision mediump float;

IN_ATTR vec2 TexCoord;
IN_ATTR vec4 Color;

OUT_ATTR vec4 FragColor;

uniform sampler2D inputTexture;

void main()
{
    vec4 sampled = texture(inputTexture, TexCoord);
    FragColor = Color * sampled;
}

#version 300 es
precision highp float;

IN_ATTR vec2 TexCoord;

uniform sampler2D inputTexture;

void main()
{
    vec4 sampled = texture(inputTexture, TexCoord);
    OUTPUT_COLOR(sampled);
}

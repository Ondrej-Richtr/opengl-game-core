#version 300 es
precision highp float;

//in vec4 gl_FragCoord;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D inputTexture1;
uniform sampler2D inputTexture2;

void main()
{
    FragColor = mix(texture(inputTexture1, TexCoord), texture(inputTexture2, TexCoord), 0.6);
}

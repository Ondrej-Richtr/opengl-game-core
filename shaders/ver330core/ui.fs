precision mediump float;

in vec2 TexCoord;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D inputTexture;

void main()
{
    vec4 sampled = texture(inputTexture, TexCoord);
    FragColor = Color * sampled;
}

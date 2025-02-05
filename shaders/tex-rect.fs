#version 300 es
precision highp float;

layout(origin_upper_left) in vec4 gl_FragCoord;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D inputTexture;
uniform vec2 rectPos;
uniform vec2 rectSize;

void main()
{
    vec2 fragScreenPos = gl_FragCoord.xy;
    vec2 texcoords = (fragScreenPos - rectPos) / rectSize;

    if (texcoords.x < 0.f || texcoords.x >= 1.f ||
        texcoords.y < 0.f || texcoords.y >= 1.f) discard;
    
    vec2 texcoords_inverted = vec2(texcoords.x, 1.0 - texcoords.y);

    vec4 sampled = texture(inputTexture, texcoords_inverted);
    FragColor = sampled;
}

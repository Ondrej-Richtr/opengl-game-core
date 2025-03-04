#version 330 core

layout (location = 0) in vec2 aPos;         // in screen relative coordinates
layout (location = 1) in vec2 aTexCoord;
layout (location = 3) in vec4 aColor;

out vec2 TexCoord;
out vec4 Color;

uniform vec2 screenRes;

void main()
{
    TexCoord = aTexCoord;
    Color = aColor;

    vec2 flipped = 2.f * (aPos / screenRes) - vec2(1.f); // in normalized coordinates with flipped y axis
    vec2 pos = vec2(flipped.x, -flipped.y);              // unflip the y axis
    gl_Position = vec4(pos, 0.f, 1.f);
}

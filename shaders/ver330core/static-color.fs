#version 330 core
precision highp float;

out vec4 FragColor;

uniform vec4 color;

void main()
{
    FragColor = color;
}

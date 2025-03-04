#version 330 core
precision highp float;

in vec4 gl_FragCoord;

out vec4 FragColor;

uniform vec4 inputColor;

void main()
{
    // float x = gl_FragCoord.x / 1280.f, y = gl_FragCoord.y / 720.f;
    // vec4 color = vec4(0.5, x, y, 1.0);
    highp int x = int(gl_FragCoord.x), y = int(gl_FragCoord.y);;
    vec4 color = (x / 60) % 2 == (y / 60) % 2 ? vec4(1.0, 0.0, 0.0, 1.0) : inputColor;
    FragColor = color;
}

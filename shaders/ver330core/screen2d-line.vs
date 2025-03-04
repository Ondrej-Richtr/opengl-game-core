#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat3 transform;
uniform vec2 screenRes;

void main()
{
    //gl_Position = projection * view * model * pos;

    vec3 pos = vec3(aPos, 1.f);
    vec3 transformed = transform * pos;
    vec2 result_screen = transformed.xy;

    vec2 result = 2.f * (result_screen / screenRes) - vec2(1.f);

    gl_Position = vec4(result, 0.f, 1.f);
}

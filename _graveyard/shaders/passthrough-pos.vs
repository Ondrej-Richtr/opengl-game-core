#version 300 es

IN_ATTR vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0f);
}

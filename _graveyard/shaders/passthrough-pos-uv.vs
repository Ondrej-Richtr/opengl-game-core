#version 300 es

IN_ATTR vec3 aPos;
IN_ATTR vec2 aTexCoord;

OUT_ATTR vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0f);
    TexCoord = aTexCoord;
}

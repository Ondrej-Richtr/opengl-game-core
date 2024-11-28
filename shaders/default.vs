#version 300 es

layout (location = 0) in vec3 aPos;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
    //vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}

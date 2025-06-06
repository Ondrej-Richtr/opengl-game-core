IN_ATTR vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    //vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}

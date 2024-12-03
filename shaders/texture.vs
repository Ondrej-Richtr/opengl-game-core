#version 300 es

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 pos = vec4(aPos, 1.0f);

    gl_Position = projection * view * model * pos;
    FragPos = vec3(model * pos);

    TexCoord = aTexCoord;
    //Normal = aNormal;
    Normal = mat3(transpose(inverse(model))) * -aNormal; //TODO check this
}

in vec3 aPos;
in vec2 aTexCoord;
in vec3 aNormal;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 model;
uniform mat3 normalMat;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 pos = vec4(aPos, 1.0f);

    gl_Position = projection * view * model * pos;
    FragPos = vec3(model * pos);

    TexCoord = aTexCoord;
    Normal = normalMat * aNormal;
}

IN_ATTR vec3 aPos;
IN_ATTR vec2 aTexCoord;
IN_ATTR vec3 aNormal;

OUT_ATTR vec3 FragPos;
OUT_ATTR vec2 TexCoord;
OUT_ATTR vec3 Normal;

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

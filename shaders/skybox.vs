IN_ATTR vec3 aPos;

OUT_ATTR vec3 TexCoord;

uniform mat3 view_stripped;
uniform mat4 projection;

void main()
{
    TexCoord = aPos;
    vec3 flipped = vec3(aPos.x, -aPos.y, aPos.z);
    vec3 view_transformed = view_stripped * flipped;
    vec4 transformed = projection * vec4(view_transformed, 1.0);
    gl_Position = transformed.xyww;
}

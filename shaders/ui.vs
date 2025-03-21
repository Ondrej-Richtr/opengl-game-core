IN_ATTR vec2 aPos;         // in screen relative coordinates
IN_ATTR vec2 aTexCoord;
IN_ATTR vec4 aColor;

OUT_ATTR vec2 TexCoord;
OUT_ATTR vec4 Color;

uniform vec2 screenRes;

void main()
{
    TexCoord = aTexCoord;
    Color = aColor;

    vec2 flipped = 2.0 * (aPos / screenRes) - vec2(1.0); // in normalized coordinates with flipped y axis
    vec2 pos = vec2(flipped.x, -flipped.y);              // unflip the y axis
    gl_Position = vec4(pos, 0.0, 1.0);
}

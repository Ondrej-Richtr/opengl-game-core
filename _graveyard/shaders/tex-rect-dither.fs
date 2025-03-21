precision highp float;

layout (origin_upper_left) IN_ATTR vec4 gl_FragCoord;

uniform sampler2D inputTexture;
uniform sampler2D inputTextureBG;
uniform sampler2D inputTextureFG;
uniform vec2 rectPos;
uniform vec2 rectSize;

#define chonk 1

#ifndef POSTPROCESS
    // #define POSTPROCESS(tpos) (vec4(0.0, 1.0, 1.0, 1.0))
    #define POSTPROCESS(tpos) (_postproc((tpos)))
    // #define POSTPROCESS(tpos) (_postproc_greyscale((tpos)))
    // #define POSTPROCESS(tpos) (texture(inputTextureBG, (tpos)))
#endif

ivec2 uvToCoord(vec2 uv)
{
    return ivec2(uv * rectSize);
}

vec2 coordToUV(ivec2 rectPos)
{
    return vec2(rectPos) / rectSize;
}

float _calculateAverage(ivec2 rectPos)
{
    vec4 c = texture(inputTexture, coordToUV(rectPos));
    return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
    // return (c.r + c.g + c.b) / 3.0;
}

int _levelFromAverage(float avg)
{
    return int(avg / 0.2f);
}

vec4 _postproc_greyscale(vec2 tpos)
{
    vec4 sampled = texture(inputTexture, tpos);
    float average = 0.2126 * sampled.r + 0.7152 * sampled.g + 0.0722 * sampled.b;
    return vec4(average, average, average, 1.0);
}

vec4 _postproc(vec2 tpos)
{
    ivec2 rectPos = uvToCoord(tpos);
    int offset = 1 << chonk;
    int mask = ~int(offset);
    ivec2 rectPosUL = rectPos & mask;
    ivec2 rectPosUR = ivec2(rectPosUL.x + offset, rectPosUL.y);
    ivec2 rectPosDL = ivec2(rectPosUL.x, rectPosUL.y + offset);
    ivec2 rectPosDR = ivec2(rectPosUL.x + offset, rectPosUL.y + offset);

    float averageVal = _calculateAverage(rectPosUL) / 4.0;
    averageVal += _calculateAverage(rectPosUR) / 4.0;
    averageVal += _calculateAverage(rectPosDL) / 4.0;
    averageVal += _calculateAverage(rectPosDR) / 4.0;

    int level = _levelFromAverage(averageVal);
    // vec4 on_color = vec4(0.8f, 0.8, 0.5, 1.0), off_color = vec4(0.0, 0.0, 0.0, 1.0);
    // vec4 on_color = vec4(105.0/255.0, 18.0/255.0, 168.0/255.0, 1.0), off_color = vec4(17.0/255.0, 45.0/255.0, 94.0/255.0, 1.0);
    // float rel_level = float(level) / 4.0;
    // vec4 off_color_min = vec4(0.1, 0.1, 0.1, 1.0), off_color_max = vec4(0.6f, 0.6f, 0.6f, 1.0);
    // vec4 on_color = texture(inputTexture, tpos), off_color = mix(off_color_min, off_color_max, rel_level);
    // vec4 on_color = texture(inputTextureFG, tpos), off_color = texture(inputTextureBG, tpos);
    vec4 on_color = vec4(0.8, 0.8, 0.5, 1.0), off_color = texture(inputTextureBG, tpos);

    if (rectPosUL == rectPos) return level >= 1 ? on_color : off_color;
    if (rectPosUR == rectPos) return level >= 4 ? on_color : off_color;
    if (rectPosDL == rectPos) return level >= 3 ? on_color : off_color;
    else return level >= 2 ? on_color : off_color;

    // return vec4(vec3(1.0) - (c).rgb, (c).a);
}

void main()
{
    //mark all input textures as used
    inputTexture;
    inputTextureBG;
    inputTextureFG;

    vec2 fragScreenPos = gl_FragCoord.xy;
    vec2 texcoords = (fragScreenPos - rectPos) / rectSize;

    //TODO use scissor test and remove this
    if (texcoords.x < 0.0 || texcoords.x >= 1.0 ||
        texcoords.y < 0.0 || texcoords.y >= 1.0) discard;
    
    vec2 texcoords_inverted = vec2(texcoords.x, 1.0 - texcoords.y);

    OUTPUT_COLOR(POSTPROCESS(texcoords_inverted));
}

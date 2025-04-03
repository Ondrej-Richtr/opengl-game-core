#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

// IN_ATTR vec4 gl_FragCoord;

uniform sampler2D inputTexture;
uniform vec2 screenRes;
uniform vec2 rectPosIn;
uniform vec2 rectSize;

#ifndef SETUP
    #define SETUP()
#endif

#ifndef POSTPROCESS
    #define POSTPROCESS(tex, tpos) (TEXTURE2D((tex), (tpos)))
#endif

void main()
{
    SETUP();

    vec2 fragScreenPos = vec2(gl_FragCoord.x, screenRes.y - gl_FragCoord.y);
    vec2 texcoords = (fragScreenPos - rectPosIn) / rectSize;

    //TODO use scissor test and remove this
    if (texcoords.x < 0.0 || texcoords.x >= 1.0 ||
        texcoords.y < 0.0 || texcoords.y >= 1.0) discard;
    
    vec2 texcoords_inverted = vec2(texcoords.x, 1.0 - texcoords.y);

    OUTPUT_COLOR(POSTPROCESS(inputTexture, texcoords_inverted));
}

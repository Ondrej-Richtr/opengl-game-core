#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

IN_ATTR vec4 gl_FragCoord;

uniform vec4 inputColor;

void main()
{
    // float x = gl_FragCoord.x / 1280.0, y = gl_FragCoord.y / 720.0;
    // vec4 color = vec4(0.5, x, y, 1.0);
    highp int x = int(gl_FragCoord.x), y = int(gl_FragCoord.y);;
    vec4 color = (x / 60) % 2 == (y / 60) % 2 ? vec4(1.0, 0.0, 0.0, 1.0) : inputColor;
    OUTPUT_COLOR(color);
}

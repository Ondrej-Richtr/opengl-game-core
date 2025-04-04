#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

//IN_ATTR vec4 gl_FragCoord;
IN_ATTR vec3 FragPos;            //position in world space
IN_ATTR vec2 TexCoord;
IN_ATTR vec3 Normal;

uniform sampler2D inputTexture1;
uniform sampler2D inputTexture2;
uniform vec3 lightSrcColor;
uniform vec3 lightSrcPos;   //position in world space
uniform vec3 cameraPos;     //position in world space

void main()
{
    vec3 norm = normalize(Normal);

    //ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightSrcColor;

    //diffuse
    vec3 lightSrcDir = normalize(lightSrcPos - FragPos);
    float diff = max(dot(norm, lightSrcDir), 0.0);
    vec3 diffuse = diff * lightSrcColor;

    //specular
    float specularStrength = 0.5;
    float specularShininess = 51.0;
    vec3 cameraDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightSrcDir, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), specularShininess);
    vec3 specular = specularStrength * spec * lightSrcColor;

    //result
    vec4 sampled = mix(TEXTURE2D(inputTexture1, TexCoord), TEXTURE2D(inputTexture2, TexCoord), 0.6);
    vec3 color = (ambient + diffuse + specular) * sampled.rgb;
    OUTPUT_COLOR(vec4(color, sampled.a));
}

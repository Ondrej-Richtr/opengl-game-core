#version 300 es
precision highp float;

//in vec4 gl_FragCoord;
in vec3 FragPos;            //position in world space
in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D inputTexture1;
uniform sampler2D inputTexture2;
uniform vec3 lightSrcColor;
uniform vec3 lightSrcPos;   //position in world space

void main()
{
    vec3 norm = normalize(Normal);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightSrcColor;

    vec3 lightSrcDir = normalize(lightSrcPos - FragPos);
    float diff = max(dot(norm, lightSrcDir), 0.0);
    vec3 diffuse = diff * lightSrcColor;

    //vec4 sampled = mix(texture(inputTexture1, TexCoord), texture(inputTexture2, TexCoord), 0.6);
    vec4 sampled = vec4(1.0, 1.0, 1.0, 1.0);
    
    //vec3 color = (ambient + diffuse) * sampled.rgb;
    vec3 color = (ambient + diffuse) * vec3(1.0, 1.0, 1.0); //DEBUG
    FragColor = vec4(color, sampled.a);
}

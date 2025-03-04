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
    float specularShininess = 51.f;
    vec3 cameraDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightSrcDir, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), specularShininess);
    vec3 specular = specularStrength * spec * lightSrcColor;

    //result
    vec4 sampled = mix(texture(inputTexture1, TexCoord), texture(inputTexture2, TexCoord), 0.6);
    vec3 color = (ambient + diffuse + specular) * sampled.rgb;
    FragColor = vec4(color, sampled.a);
}

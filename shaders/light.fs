#version 300 es
precision highp float;

struct Material
{
    vec3 ambient; //TODO same as texture in texture.fs
    vec3 diffuse; //TODO same as texture in texture.fs
    vec3 specular;
    float shininess;
};

struct LightSrc
{
    vec3 pos;               //position in world space

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//in vec4 gl_FragCoord;
in vec3 FragPos;            //position in world space
in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

//uniform sampler2D inputTexture;
uniform vec3 cameraPos;     //position in world space
uniform Material material;
uniform LightSrc lightSrc;

void main()
{
    vec3 norm = normalize(Normal);

    //ambient
    vec3 ambient = material.ambient * lightSrc.ambient;

    //diffuse
    vec3 lightSrcDir = normalize(lightSrc.pos - FragPos);
    float diff = max(dot(norm, lightSrcDir), 0.0);
    vec3 diffuse = diff * (material.diffuse * lightSrc.diffuse);

    //specular
    vec3 cameraDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightSrcDir, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * (material.specular * lightSrc.specular);

    //result
    // vec4 sampled = mix(texture(inputTexture1, TexCoord), texture(inputTexture2, TexCoord), 0.6);
    // vec3 color = (ambient + diffuse + specular) * sampled.rgb;
    // FragColor = vec4(color, sampled.a);
    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}

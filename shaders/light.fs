#version 300 es
precision highp float;

#define LIGHTS_MAX_AMOUNT 10 //TODO implement includes

struct Material
{
    vec3 ambient; //TODO same as texture in texture.fs
    vec3 diffuse; //TODO same as texture in texture.fs
    vec3 specular;
    float shininess;
};

struct LightProps
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Light
{
    LightProps props;
    int type;        // 0 -> directional, 1 -> point light, 2 -> spot light

    vec3 pos;        // only for point and spot lights
    vec3 dir;        // only for directional and spot lights
    float cosCutoff; // only for spot lights
};

//in vec4 gl_FragCoord;
in vec3 FragPos;            //position in world space
in vec2 TexCoord;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D inputTexture;
uniform vec3 cameraPos;     //position in world space
uniform Material material;
uniform Light lights[LIGHTS_MAX_AMOUNT];
uniform int lightsCount;

vec3 calc_dir_light(vec3 norm, vec3 cameraDir, LightProps props, vec3 dir)
{
    vec3 result = vec3(0.f);
    vec3 lightDir = normalize(-dir);

    //ambient
    result += material.ambient * props.ambient;

    //diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    result += diff * (material.diffuse * props.diffuse);

    //specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    result += spec * (material.specular * props.specular);

    return result;
}

vec3 calc_point_light(vec3 norm, vec3 cameraDir, LightProps props, vec3 lightPos)
{
    //TODO
    vec3 result = vec3(0.f);

    //ambient
    result += material.ambient * props.ambient;

    //diffuse
    vec3 lightSrcDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightSrcDir), 0.0);
    result += diff * (material.diffuse * props.diffuse);

    //specular
    vec3 reflectDir = reflect(-lightSrcDir, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    result += spec * (material.specular * props.specular);

    return result;
}

vec3 calc_spot_light()
{
    //TODO
    return vec3(0.f);
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 cameraDir = normalize(cameraPos - FragPos);
    
    //light
    vec3 lightColor = vec3(0.f);

    for (int i = 0; i < lightsCount; ++i)
    {
        switch (lights[i].type)
        {
        case 0: // lights[i] is a directional light
            lightColor += calc_dir_light(norm, cameraDir, lights[i].props, lights[i].dir);
            break;
        case 1: // lights[i] is a point light
            lightColor += calc_point_light(norm, cameraDir, lights[i].props, lights[i].pos);
            break;
        case 2: // lights[i] is a spot light
            lightColor += calc_spot_light(); //TODO
            break;
        }
    }

    //result
    vec4 sampled = texture(inputTexture, TexCoord);
    vec3 color = lightColor * sampled.rgb;
    FragColor = vec4(color, sampled.a);
}

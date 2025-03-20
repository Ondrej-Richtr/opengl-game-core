precision highp float;

// #define LIGHTS_MAX_AMOUNT 10
// #define ALPHA_MIN_THRESHOLD 0.35f // 0.35f looks good

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
    int type;               // 0 -> directional, 1 -> point light, 2 -> spot light

    vec3 atten_coefs;       // coeficients for calculating attenuation - in order: constant, linear, quadratic, only for point and spot lights

    vec3 pos;               // only for point and spot lights
    vec3 dir;               // only for directional and spot lights
    float cosInnerCutoff;   // only for spot lights
    float cosOuterCutoff;   // only for spot lights
};

//in vec4 gl_FragCoord;
IN_ATTR vec3 FragPos;            //position in world space
IN_ATTR vec2 TexCoord;
IN_ATTR vec3 Normal;

uniform sampler2D inputTexture;
uniform vec3 cameraPos;     //position in world space
uniform Material material;
uniform Light lights[LIGHTS_MAX_AMOUNT]; //TODO this might not work everywhere!
uniform int lightsCount;

vec3 calc_dir_light(vec3 norm, vec3 cameraDir, LightProps props, vec3 dir)
{
    vec3 result = vec3(0.0);
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

vec3 calc_point_light(vec3 norm, vec3 cameraDir, LightProps props, vec3 lightPos, vec3 atten_coefs)
{
    vec3 result = vec3(0.0);

    //ambient
    result += material.ambient * props.ambient;

    //diffuse
    vec3 dirToLight = normalize(lightPos - FragPos);
    float diff = max(dot(norm, dirToLight), 0.0);
    result += diff * (material.diffuse * props.diffuse);

    //specular
    vec3 reflectDir = reflect(-dirToLight, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    result += spec * (material.specular * props.specular);

    //attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 /
                        (atten_coefs.x +                           // constant component
                         atten_coefs.y *  distance +               // linear component
                         atten_coefs.z * (distance * distance));   // quadratic component
    result *= attenuation;

    return result;
}

vec3 calc_spot_light(vec3 norm, vec3 cameraDir, LightProps props, vec3 lightDir, vec3 lightPos,
                     float cosCutoffIn, float cosCutoffOut, vec3 atten_coefs)
{
    vec3 result = vec3(0.0);
    vec3 dirToLight = normalize(lightPos - FragPos); // direction of the light source from the fragment

    float cosTheta   = dot(dirToLight, normalize(-lightDir)); // -lightDir as we have directions from the fragment
    // intensity of the light - 1.0 for inner cone (full intensity), 0.0 for fragments out of both cones (no intensity), 0.0-1.0 in the outer cone
    float intensity  = clamp((cosTheta - cosCutoffOut) / (cosCutoffIn - cosCutoffOut), 0.0, 1.0);

    //ambient
    result += material.ambient * props.ambient; // intensiity is NOT applied for ambient light

    //diffuse
    float diff = max(dot(norm, dirToLight), 0.0);
    result += (intensity * diff) * (material.diffuse * props.diffuse); // apply intensity

    //specular
    vec3 reflectDir = reflect(-dirToLight, norm);
    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    result += (intensity * spec) * (material.specular * props.specular); // also apply intensity

    //attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 /
                        (atten_coefs.x +                           // constant component
                         atten_coefs.y *  distance +               // linear component
                         atten_coefs.z * (distance * distance));   // quadratic component
    result *= attenuation;

    return result;
}

void main()
{
    vec4 sampled = texture(inputTexture, TexCoord);
    // discard the fragments with too small alpha values
    //TODO aplha blending
    if (sampled.a < ALPHA_MIN_THRESHOLD) discard;

    vec3 norm = normalize(Normal);
    vec3 cameraDir = normalize(cameraPos - FragPos);
    
    //light
    vec3 lightColor = vec3(0.0);

    for (int i = 0; i < lightsCount; ++i)
    {
        switch (lights[i].type)
        {
        case 0: // lights[i] is a directional light
            lightColor += calc_dir_light(norm, cameraDir, lights[i].props, lights[i].dir);
            break;
        case 1: // lights[i] is a point light
            lightColor += calc_point_light(norm, cameraDir, lights[i].props, lights[i].pos, lights[i].atten_coefs);
            break;
        case 2: // lights[i] is a spot light
            lightColor += calc_spot_light(norm, cameraDir, lights[i].props, lights[i].dir, lights[i].pos,
                                          lights[i].cosInnerCutoff, lights[i].cosOuterCutoff, lights[i].atten_coefs);
            break;
        }
    }

    //result
    vec3 color = lightColor * sampled.rgb;
    OUTPUT_COLOR(vec4(color, sampled.a));
}

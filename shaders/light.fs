#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    sampler2D diffuseMap;
    vec3 specular;
    sampler2D specularMap;
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

IN_ATTR vec3 FragPos;       //position in world space
IN_ATTR vec2 TexCoord;
IN_ATTR vec3 Normal;

uniform vec3 cameraPos;     //position in world space
uniform Material material;
uniform Light lights[LIGHTS_MAX_AMOUNT]; //TODO this might not work everywhere!
uniform int lightsCount;
uniform float gammaCoef;

const bool use_blinn = true; // use blinn variant of Phong shading model
const float diff_cutoff_for_spec = 0.05;
//TODO make a uniform to enable/disable this setting
const bool use_reinhard = true; // use reinhard tone mapping

vec3 calc_dir_light(vec3 norm, vec3 cameraDir, vec3 dir)
{
    vec3 lightDir = normalize(-dir);

    //ambient
    float amb = 1.0;

    //diffuse
    float diff = max(dot(norm, lightDir), 0.0);

    //specular
    float spec = 0.0;
    if (use_blinn)
    {
        if (diff > diff_cutoff_for_spec) //TODO probably remove this after shadows gets implemented (causes ball to look weird)
        {
            vec3 halfwayDir = normalize(lightDir + cameraDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        }
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    }

    return vec3(amb, diff, spec);
}

vec3 calc_point_light(vec3 norm, vec3 cameraDir, vec3 lightPos, vec3 atten_coefs)
{
    //ambient
    float amb = 1.0;

    //diffuse
    vec3 dirToLight = normalize(lightPos - FragPos);
    float diff = max(dot(norm, dirToLight), 0.0);

    //specular
    float spec = 0.0;
    if (use_blinn)
    {
        if (diff > diff_cutoff_for_spec) //TODO probably remove this after shadows gets implemented (causes ball to look weird)
        {
            vec3 halfwayDir = normalize(dirToLight + cameraDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        }
    }
    else
    {
        vec3 reflectDir = reflect(-dirToLight, norm);
        spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess);
    }

    //attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 /
                        (atten_coefs.x +                           // constant component
                         atten_coefs.y *  distance +               // linear component
                         atten_coefs.z * (distance * distance));   // quadratic component

    return vec3(amb, diff, spec) * attenuation;
}

vec3 calc_spot_light(vec3 norm, vec3 cameraDir, vec3 lightDir, vec3 lightPos,
                     float cosCutoffIn, float cosCutoffOut, vec3 atten_coefs)
{
    vec3 dirToLight = normalize(lightPos - FragPos); // direction of the light source from the fragment

    float cosTheta = dot(dirToLight, normalize(-lightDir)); // -lightDir as we have directions from the fragment
    // intensity of the light - 1.0 for inner cone (full intensity), 0.0 for fragments out of both cones (no intensity), 0.0-1.0 in the outer cone
    float intensity = clamp((cosTheta - cosCutoffOut) / (cosCutoffIn - cosCutoffOut), 0.0, 1.0);

    //ambient
    float amb = 1.0; // intensity is NOT applied for ambient light

    //diffuse
    float diff = max(dot(norm, dirToLight), 0.0) * intensity; // intensity applied

    //specular
    float spec = 0.0;
    if (use_blinn)
    {
        if (diff > diff_cutoff_for_spec) //TODO probably remove this after shadows gets implemented (causes ball to look weird)
        {
            vec3 halfwayDir = normalize(dirToLight + cameraDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess) * intensity;  // intensity applied
        }
    }
    else
    {
        vec3 reflectDir = reflect(-dirToLight, norm);
        spec = pow(max(dot(cameraDir, reflectDir), 0.0), material.shininess) * intensity; // intensity applied
    }

    //attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 /
                        (atten_coefs.x +                           // constant component
                         atten_coefs.y *  distance +               // linear component
                         atten_coefs.z * (distance * distance));   // quadratic component

    return vec3(amb, diff, spec) * attenuation;
}

vec3 tone_mapping(vec3 c)
{
    // optional reinhard tone mapping
    return use_reinhard ? c / (c + vec3(1.0)) : c;
}

void main()
{
    vec4 diffuse_sample = TEXTURE2DGAMMA(material.diffuseMap, TexCoord);
    // discard the fragments with too small alpha values
    //TODO aplha blending
    if (diffuse_sample.a < ALPHA_MIN_THRESHOLD) discard;

    vec3 specular_sample = TEXTURE2D(material.specularMap, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    vec3 cameraDir = normalize(cameraPos - FragPos);
    
    //light
    vec3 color = vec3(0.0);

    for (int i = 0; i < LIGHTS_MAX_AMOUNT; ++i)
    {
        //NOTE this might seem strange, but checking the bounds inside the for loop condition does not work with glsl version 100!
        if (i >= lightsCount) break;

        vec3 phong_light_coefs = vec3(0.0);
        if (lights[i].type == 0) // lights[i] is a directional light
        {
            phong_light_coefs = calc_dir_light(norm, cameraDir, lights[i].dir);
        }
        else if (lights[i].type == 1) // lights[i] is a point light
        {
            phong_light_coefs = calc_point_light(norm, cameraDir, lights[i].pos, lights[i].atten_coefs);
        }
        else if (lights[i].type == 2) // lights[i] is a spot light
        {
            phong_light_coefs = calc_spot_light(norm, cameraDir, lights[i].dir, lights[i].pos,
                                                lights[i].cosInnerCutoff, lights[i].cosOuterCutoff, lights[i].atten_coefs);
        }

        color += phong_light_coefs.x * lights[i].props.ambient * material.ambient * diffuse_sample.rgb; // ambient
        color += phong_light_coefs.y * lights[i].props.diffuse * material.diffuse * diffuse_sample.rgb; // diffuse
        color += phong_light_coefs.z * lights[i].props.specular * material.specular * specular_sample;  // specular
    }

    //result
    vec3 mapped = tone_mapping(color);
    vec4 result = vec4(mapped, diffuse_sample.a);
    OUTPUT_COLOR_GAMMA_CORRECTED(result);
}

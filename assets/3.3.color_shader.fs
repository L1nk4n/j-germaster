#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int shininess;
};

struct pointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct directionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct flashLight {
    float innerCutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform vec3 viewPos;
uniform vec3 cViewDir;
uniform sampler2D texture1;

uniform Material material;

#ifdef POINTLIGHT
uniform pointLight pointLights[POINTLIGHT];
#endif

#ifdef DIRLIGHT
uniform directionalLight dirLights[DIRLIGHT];
#endif

#ifdef FLASHLIGHT
uniform flashLight flashLights[FLASHLIGHT];
#endif

vec3 computeDirectionalLight(directionalLight lightSource);
vec3 computePointLight(pointLight lightSource);
vec3 computeFlashLight(flashLight lightSource);

void main()
{
    vec4 color = texture(texture1, TexCoord);
    vec3 light = vec3(0.0);

    #ifdef DIRLIGHT
    for (int i = 0; i < DIRLIGHT; i++) {
        light += computeDirectionalLight(dirLights[i]);
    }
    #endif

    #ifdef POINTLIGHT
    for (int i = 0; i < POINTLIGHT; i++) {
        light += computePointLight(pointLights[i]);
    }
    #endif

    #ifdef FLASHLIGHT
    for (int i = 0; i < FLASHLIGHT; i++) {
        light += computeFlashLight(flashLights[i]);
    }
    #endif

    light = clamp(light, 0.0, 1.0);
 
    vec4 result = vec4(light, 1) * color;
    FragColor = result;
}

vec3 computeDirectionalLight(directionalLight lightSource) {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightSource.direction);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightSource.diffuse * (diff * material.diffuse);

    vec3 ambient = lightSource.ambient * material.ambient;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightSource.specular * (spec * material.specular);
 
    return ambient + diffuse + specular;
}

vec3 computePointLight(pointLight lightSource) {
    float distance    = length(lightSource.position - FragPos);
    float attenuation = 1.0 / (lightSource.constant + lightSource.linear * distance + lightSource.quadratic * (distance * distance)); 

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightSource.position - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightSource.diffuse * (diff * material.diffuse);

    vec3 ambient = lightSource.ambient * material.ambient;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    //vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = lightSource.specular * (spec * material.specular);
 
    return attenuation * (ambient + diffuse + specular);
}

vec3 computeFlashLight(flashLight lightSource) {
    // float distance    = length(viewPos - FragPos);
    // float attenuation = 1.0 / (lightSource.constant + lightSource.linear * distance + lightSource.quadratic * (distance * distance)); 

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(viewPos - FragPos);

    float theta = dot(lightDir, normalize(-cViewDir));
    float epsilon = lightSource.innerCutOff - lightSource.outerCutOff;
    float intensity = clamp((theta - lightSource.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 ambient  = lightSource.ambient  * material.ambient;
    vec3 diffuse  = lightSource.diffuse  * (diff * material.diffuse);

    vec3 viewDir    = normalize(viewPos - FragPos);
    //vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec      = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular   = lightSource.specular * (spec * material.specular);

    return intensity * (ambient + diffuse + specular);
}

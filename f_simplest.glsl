#version 330 core


out vec4 FragColor;


in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;


uniform sampler2D texStageFloor;
uniform sampler2D texStageFloorSpec;
uniform vec3 lightPos[6];
uniform vec4 lightColors[6];
uniform vec3 camPos;
uniform int enLight;
vec4 spotLight(int n)
{
    float outerCone = 0.90f;
    float innerCone = 0.95f;

    float ambient = 0.20f;
    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(lightPos[n] - crntPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.50f;
    vec3 viewDirection = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;


    float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
    float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

    vec3 texColor = texture(texStageFloor, texCoord).rgb;
    vec3 specColor = texture(texStageFloorSpec, texCoord).rgb;

    vec3 ambientVec = ambient * texColor * inten * vec3(lightColors[n]);
    vec3 diffuseVec = diffuse * texColor * inten * vec3(lightColors[n]);
    vec3 specVec = specular * specColor * inten * vec3(lightColors[n]);
    return vec4(ambientVec + diffuseVec + specVec, 1.0);
}

vec4 directionalLight()
{
    vec3 directionalLightColor = vec3(1.0, 1.0, 1.0); 
    vec3 directionalLightDirection = normalize(vec3(0, 0, 0)); 
    vec3 ambientLightColor = vec3(0.3, 0.3, 0.3); 

    float ambient = 0.80f;

    vec3 normal = normalize(Normal);
    vec3 lightDirection = normalize(directionalLightDirection);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    float specularLight = 0.50f;
    vec3 viewDirection = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;

    vec3 texColor = texture(texStageFloor, texCoord).rgb;
    vec3 specColor = texture(texStageFloorSpec, texCoord).rgb;

    vec3 ambientVec = ambient * texColor * ambientLightColor;
    vec3 diffuseVec = diffuse * texColor * directionalLightColor;
    vec3 specVec = specular * specColor * directionalLightColor;
    return vec4(ambientVec + diffuseVec + specVec, 1.0);
}

void main()
{
    vec3 result = vec3(0.0);
    for(int i = 0; i < 6; i++)
        result += spotLight(i).rgb; 
    if (enLight == 1) {
        result += directionalLight().rgb; 
    }
    
    result = clamp(result, 0.0, 1.0); 
    FragColor = vec4(result, 1.0);
}

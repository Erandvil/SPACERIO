#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform sampler2D diffuse; 
uniform sampler2D specularTex;
uniform sampler2D normalMap;

uniform vec3 lightColor;
uniform float shininess;

void main() 
{
    // 1. Normalna z mapy normalnych
    vec3 normal = texture(normalMap, TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // 2. Kolor bazowy i Ambient
    float ambientStrength = 0.1f; 
    vec3 color = texture(diffuse, TexCoord).rgb;
    vec3 ambient = lightColor * ambientStrength * color;

    // 3. Diffuse (światło rozproszone)
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseCol = lightColor * diff * color;

    // 4. Specular (odblaski - głównie na wodzie)
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos); 
    vec3 reflectDir = reflect(-lightDir, normal);
    
    // Blinn-Phong (opcjonalnie dla lepszego efektu)
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    float specMap = texture(specularTex, TexCoord).r;
    vec3 specular = lightColor * spec * specMap;

    FragColor = vec4(ambient + diffuseCol + specular, 1.0);
}
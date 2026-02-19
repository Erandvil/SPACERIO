#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform sampler2D ourTexture;
uniform vec3 lightColor;
uniform float shininess;
uniform sampler2D specularTex;
uniform sampler2D normalMap;

void main() 
{
    // 1. Normalna z mapy
    vec3 normal = texture(normalMap, TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // 2. Kolor bazowy i Ambient
    float ambientStrength = 0.05f; // Trochę zwiększyłem, żeby nie było czarnej plamy
    vec3 color = texture(ourTexture, TexCoord).rgb;
    vec3 ambient = lightColor * ambientStrength * color;

    // 3. Diffuse
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * color;

    // 4. Specular - POPRAWIONE viewDir
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos); 
    vec3 reflectDir = reflect(-lightDir, normal);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float specMap = texture(specularTex, TexCoord).r;
    vec3 specular = lightColor * spec * specMap * 0.5f;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
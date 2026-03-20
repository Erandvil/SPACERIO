#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normals;
in vec3 FragPos;

uniform sampler2D ourTexture;
uniform samplerCube depthMap;
uniform float far_plane;

uniform vec3 lightPos;
uniform vec3 lightColor;

float ShadowCalculation(vec3 fragPos)
{
   vec3 fragToLight = fragPos - lightPos;
   float closetsDepth = texture(depthMap, fragToLight).r;
   closetsDepth *= far_plane;
   float currentDepth = length(fragToLight);

   float bias = 0.05;
   float shadow = currentDepth - bias > closetsDepth ? 1.0 : 0.0;
   return shadow;
}

void main() 
{
   vec3 color = texture(ourTexture, TexCoord).rgb;
   float ambientStrength = 0.2f;
   vec3 norm = normalize(Normals);
   vec3 lightDir = normalize(lightPos - FragPos);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 ambient = lightColor * ambientStrength;
   vec3 diffuse = diff * lightColor;

   float shadow = ShadowCalculation(FragPos);
   vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color;

   FragColor = vec4(lighting, 1.0);
}
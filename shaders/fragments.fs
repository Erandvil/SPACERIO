#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normals;
in vec3 FragPos;

uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main() 
{
   float ambientStrength = 0.2f;
   vec3 norm = normalize(Normals);
   vec3 lightDir = normalize(lightPos - FragPos);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 ambient = lightColor * ambientStrength;
   vec3 diffuse = diff * lightColor;
   FragColor = vec4(ambient + diffuse, 1.0) * texture(ourTexture, TexCoord);
}
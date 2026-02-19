#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

out vec2 TexCoord;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

void main()
{
   vec3 FragPos = vec3(model * vec4(aPos, 1.0));
   TexCoord = aTexCoords;

   mat3 normalMatrix = transpose(inverse(mat3(model)));

   vec3 T = normalize(normalMatrix * aTangent);
   vec3 N = normalize(normalMatrix * aNormals);

   T = normalize(T - dot(T, N) * N);
   vec3 B = cross(N, T);

   mat3 TBN = transpose(mat3(T, B, N));

   TangentLightPos = TBN * lightPos;
   TangentViewPos = TBN * viewPos;
   TangentFragPos = TBN * FragPos;

   gl_Position = projection * view * model * vec4(aPos, 1.0);
}
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 Normals;
out vec3 FragPos;

void main()
{
   gl_Position = projection * view * model * vec4(aPos, 1.0);
   TexCoord = aTexCoords;
   Normals = mat3(transpose(inverse(model))) * aNormals;
   FragPos = vec3(model * vec4(aPos, 1.0));
}
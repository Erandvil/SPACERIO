#version 330 core
layout (location = 0) in vec3 squareVertices; // Wierzchołki kwadratu (od -0.5 do 0.5)
layout (location = 1) in vec4 xyzs;           // Pozycja (xyz) i skala (s) z VBO instancji
layout (location = 2) in vec4 color;          // Kolor z VBO instancji

out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

void main()
{
    float particleSize = xyzs.w;
    vec3 particleCenter = xyzs.xyz;

    vec3 vertexPosition_worldspace = 
        particleCenter
        + cameraRight * squareVertices.x * particleSize
        + cameraUp * squareVertices.y * particleSize;

    gl_Position = projection * view * vec4(vertexPosition_worldspace, 1.0);

    ParticleColor = color;
}
#include <iostream>
#include <map>
#include <vector>
#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "include/imgui/imgui.h"
#include "include/imgui/backends/imgui_impl_glfw.h"
#include "include/imgui/backends/imgui_impl_opengl3.h"

#include "include/shader.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void buildSphere(int sectorCount, int stackCount, float radius);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(const char* path);
unsigned int loadCubeMap(std::vector<std::string> faces);
void key_callback(GLFWwindow* window, int key, int scancodem, int action, int mods);

int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;

float FOV = 45.0f;

float PI = 3.14159265359f;

std::vector<float> vertices;
std::vector<unsigned int> indices;
std::vector<int> lineIndices;
std::vector<int> texCoords;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = 400.0f;
float lastY = 300.0f;

const float sensitivity = 0.1f;

float yaw = -90.0f;
float pitch = 0.0f;

bool firstMouse = true;

float PI_NUMBER = 3.14159;

float MERCURY_ORBIT_RADIUS = 2.0f;
float VENUS_ORBIT_RADIUS = 4.0f;
float EARTH_ORBIT_RADIUS = 6.0f;
float MARS_ORBIT_RADIUS = 8.0f;
float JUPITER_ORBIT_RADIUS = 10.0f;
float SATURN_ORBIT_RADIUS = 12.0f;
float URANUS_ORBIT_RADIUS = 14.0f;
float NEPTUNE_ORBIT_RADIUS = 16.0f;

float MERCURY_SCALE = 0.3f;
float VENUS_SCALE = 0.3f;
float EARTH_SCALE = 0.3f;
float MARS_SCALE = 0.3f;
float JUPITER_SCALE = 0.3f;
float SATURN_SCALE = 0.3f;
float URANUS_SCALE = 0.3f;
float NEPTUNE_SCALE = 0.3f;

float MERCURY_ORBIT_TIME = 0.24f;
float WENUS_ORBIT_TIME = 0.62f;
float EARTH_ORBIT_TIME = 1.0f;
float MARS_ORBIT_TIME = 1.88f;
float JUPITER_ORBIT_TIME = 12.0f;
float SATURN_ORBIT_TIME = 29.5f;
float URANUS_ORBIT_TIME = 84.0f;
float NEPTUN_ORBIT_TIME = 165.0f;

float TIME_MULTIPLER = 0.5f;

bool showPanel = false;
bool showPlanetZoom = false;

struct Particle 
{
    glm::vec3 pos, speed;
    glm::vec4 color;
    float life;

    Particle() : pos(0.0f), speed(0.0f), color(1.0f, 0.5f, 0.1f, 1.0f), life(-1.0f){}
};

const int MAX_PARTICLES = 100;

Particle particlesContainer[MAX_PARTICLES];

float particle_pos_data[MAX_PARTICLES * 4];
float particle_col_data[MAX_PARTICLES * 4];

struct Planet
{
    std::string name;
    unsigned int diffuseTexture;
    unsigned int specularTexture;
    unsigned int normalTexture;

    float orbitRadius;
    float orbitTime;
    float scale;
    float selfRotationSpeed;
    float rotationAngle;

    Shader* shader;
};

std::map <std::string, Planet> planets;

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Spacerio", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to load GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);\
    glfwSetKeyCallback(window, key_callback);

    // OPENGL TESTING
    glEnable(GL_DEPTH_TEST);

    // IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // IMGUI END

    Shader basic_planet_shader("shaders/vertex.vs", "shaders/fragments.fs");
    Shader sun_shader("shaders/sun.vs", "shaders/sun.fs");
    Shader earth_shader("shaders/earth.vs", "shaders/earth.fs");
    Shader skybox_shader("shaders/cubemap.vs", "shaders/cubemap.fs");
    Shader particle_shader("shaders/particle.vs", "shaders/particle.fs");

    buildSphere(36, 18, 1.0f);

    static const float particle_vertices[] = 
    {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };

    unsigned int particles_VAO;
    glGenVertexArrays(1, &particles_VAO);
    glBindVertexArray(particles_VAO);

    // Square vertices
    glEnableVertexAttribArray(0);
    unsigned int particles_bilboard_VBO;
    glGenBuffers(1, &particles_bilboard_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, particles_bilboard_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_vertices), particle_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Position and size of particles
    glEnableVertexAttribArray(1);
    unsigned int particles_position_VBO;
    glGenBuffers(1, &particles_position_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_VBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Color of the particles
    glEnableVertexAttribArray(2);
    unsigned int particles_color_VBO;
    glGenBuffers(1, &particles_color_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_VBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1); // Pozycja jedna na każdą cząsteczkę
    glVertexAttribDivisor(2, 1); // Kolor jeden na każdą cząsteczke

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    
    unsigned int skybox_VAO;
    glGenVertexArrays(1, &skybox_VAO);
    glBindVertexArray(skybox_VAO);
    unsigned int skybox_VBO;
    glGenBuffers(1, &skybox_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(FOV, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1f, 100.0f);
    
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    
    unsigned int sun_texture = loadTexture("resources/planets/sun_planet.jpg");
    unsigned int earth_texture = loadTexture("resources/planets/earth_planet.jpg");
    unsigned int earth_specular_map_texture = loadTexture("resources/planets/earthspec1k.jpg");
    unsigned int earth_normals_texture = loadTexture("resources/planets/earth_planet_normals.jpg");
    unsigned int moon_texture = loadTexture("resources/planets/moon_planet.jpg");
    unsigned int mercury_texture = loadTexture("resources/planets/mercury.jpg");
    unsigned int venus_texture = loadTexture("resources/planets/venus.jpg");
    unsigned int mars_texture = loadTexture("resources/planets/mars.jpg");
    unsigned int jupiter_texture = loadTexture("resources/planets/jupiter.jpg");
    unsigned int saturn_texture = loadTexture("resources/planets/saturn.jpg");
    unsigned int uranus_texture = loadTexture("resources/planets/uranus.jpg");
    unsigned int neptun_texture = loadTexture("resources/planets/neptune.jpg");
    
    std::vector<std::string> cube_map_paths = {
        "resources/skybox/px.jpg", // GL_TEXTURE_CUBE_MAP_POSITIVE_X (Prawa) 
        "resources/skybox/nx.jpg", // GL_TEXTURE_CUBE_MAP_NEGATIVE_X (Lewa)
        "resources/skybox/py.jpg", // GL_TEXTURE_CUBE_MAP_POSITIVE_Y (Góra)
        "resources/skybox/ny.jpg", // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (Dół)
        "resources/skybox/pz.jpg", // GL_TEXTURE_CUBE_MAP_POSITIVE_Z (Tył/Przód - zależnie od układu)
        "resources/skybox/nz.jpg"  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (Przód/Tył - zależnie od układu)
    };
    unsigned int cubemapTexture = loadCubeMap(cube_map_paths);

    glm::vec3 lightPos = glm::vec3(0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f);

    planets["Mercury"] = 
    {
        "MERCURY",
        mercury_texture, 0, 0,
        2.0f, 0.24f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };

    planets["Venus"] = 
    {
        "VENUS",
        venus_texture, 0, 0,
        4.0f, 0.62f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };

    planets["Mars"] = 
    {
        "MARS",
        mars_texture, 0, 0,
        8.0f, 1.88f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };

    planets["Jupiter"] = 
    {
        "JUPITER",
        jupiter_texture, 0, 0,
        10.0f, 12.0f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };

    planets["Saturn"] = 
    {
        "SATURN",
        saturn_texture, 0, 0,
        12.0f,29.5f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };

    planets["Uranus"] = 
    {
        "URANUS",
        uranus_texture, 0, 0,
        14.0f,84.0f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };

    planets["Neptun"] = 
    {
        "NEPTUN",
        neptun_texture, 0, 0,
        16.0f, 165.0f, 0.3f, 0.0f, 0.0f,
        &basic_planet_shader
    };
    
    while (!glfwWindowShouldClose(window))
    {
        
        processInput(window);
        
        int alive_particles = 0;
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            Particle& p = particlesContainer[i];

            if (p.life > 0.0f)
            {
                p.life -= deltaTime;
                if (p.life > 0.0f)
                {
                    p.pos += p.speed * deltaTime;

                    particle_pos_data[4 * alive_particles + 0] = p.pos.x;
                    particle_pos_data[4 * alive_particles + 1] = p.pos.y;
                    particle_pos_data[4 * alive_particles + 2] = p.pos.z;
                    particle_pos_data[4 * alive_particles + 3] = 0.15f * (p.life * 0.2f);

                    particle_col_data[4 * alive_particles + 0] = p.color.r;
                    particle_col_data[4 * alive_particles + 1] = p.color.g;
                    particle_col_data[4 * alive_particles + 2] = p.color.b;
                    particle_col_data[4 * alive_particles + 3] = p.life;

                    alive_particles++;
                }
            }
            else
            {
                p.life = 1.0f;
                p.pos = glm::vec3(0.0f);
                float angle = (rand() % 360) * 3.14f / 180.0f;
                float speedMag = (rand() % 100) / 100.0f * 0.5f + 0.2f;
                p.speed = glm::vec3(cos(angle), (rand() % 100) / 50.0f - 1.0f, sin(angle));
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, particles_position_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, alive_particles * 4 * sizeof(float), particle_pos_data);

        glBindBuffer(GL_ARRAY_BUFFER, particles_color_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, alive_particles * 4 * sizeof(float), particle_col_data);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        float current_fps = 1.0f/ deltaTime;
        std::string windowTitle = "Spacerio " + std::to_string(current_fps);
        glfwSetWindowTitle(window, windowTitle.c_str());
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDepthMask(GL_FALSE);
        skybox_shader.use();
        skybox_shader.setInt("skybox", 0);
        skybox_shader.setMat4("projection", projection);
        glm::mat4 skybox_view = glm::mat4(glm::mat3(view));
        skybox_shader.setMat4("view", skybox_view);
        
        glBindVertexArray(skybox_VAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);

        float CURRENT_TIME = (float)glfwGetTime() * TIME_MULTIPLER;
        
        glm ::mat4 sun_model = glm::mat4(1.0f);
        glm::mat4 earth_model = glm::mat4(1.0f);
        glm::mat4 moon_model = glm::mat4(1.0f);
        
        float rotation = float(glfwGetTime()) * 0.1f;
        sun_model = glm::rotate(sun_model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); 
        
        sun_shader.use();
        sun_shader.setMat4("model", sun_model);
        sun_shader.setMat4("projection", projection);
        sun_shader.setMat4("view", view);
        sun_shader.setInt("ourTexture", 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sun_texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        
        earth_shader.use();
        earth_shader.setVec3("lightPos", lightPos);
        earth_shader.setVec3("lightColor", lightColor);
        earth_shader.setMat4("projection", projection);
        earth_shader.setMat4("view", view);
        earth_shader.setInt("ourTexture", 0);
        earth_shader.setInt("specularTex", 1);
        earth_shader.setInt("normalMap", 2);
        earth_shader.setFloat("shininess", 2);
        earth_shader.setVec3("viewPos", cameraPos);
        float rotation_around_sun = (float)glfwGetTime() * 0.2f;
        float rotation_around_own_axes = (float)glfwGetTime() * 0.8f;
        earth_model = glm::rotate(earth_model, rotation_around_sun, glm::vec3(0.0f , 1.0f, 0.0f));
        earth_model = glm::translate(earth_model, glm::vec3(EARTH_ORBIT_RADIUS, 0.0f, 0.0f));
        earth_model = glm::scale(earth_model, glm::vec3(EARTH_SCALE));
        earth_model = glm::rotate(earth_model, rotation_around_own_axes, glm::vec3(0.0f, 1.0f, 0.0f));

        earth_shader.setMat4("model", earth_model);

        if (showPlanetZoom)
        {
            glm::vec3 copy_of_earth_model = earth_model[3];
            copy_of_earth_model.z += 1.0f;
            cameraPos = copy_of_earth_model;
            cameraFront.x = 0.0f;
            cameraFront.z = -1.0f;
            cameraFront.y = 0.0f;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earth_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, earth_specular_map_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earth_normals_texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        
        basic_planet_shader.use();
        basic_planet_shader.setVec3("lightPos", lightPos);
        basic_planet_shader.setVec3("lightColor", lightColor);
        basic_planet_shader.setMat4("projection", projection);
        basic_planet_shader.setMat4("view", view);
        basic_planet_shader.setInt("ourTexture", 0);
        
        float moon_rotation_earth = (float)glfwGetTime() * 0.8f;
        moon_model = glm::rotate(earth_model, moon_rotation_earth, glm::vec3(0.0f , 1.0f, 0.0f));
        moon_model = glm::translate(moon_model, glm::vec3(2.0f, 0.0f, 0.0f));
        moon_model = glm::scale(moon_model, glm::vec3(0.3f));
        basic_planet_shader.setMat4("model", moon_model);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moon_texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        for (auto& [name, planet] : planets)
        {
            planet.shader->use();
            
            planet.rotationAngle = (2.0f * PI_NUMBER / planet.orbitTime) * CURRENT_TIME;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, planet.rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate(model, glm::vec3(planet.orbitRadius, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(planet.scale));

            planet.shader->setMat4("model", model);
            planet.shader->setMat4("projection", projection);
            planet.shader->setMat4("view", view);
            planet.shader->setVec3("lightPos", lightPos);
            planet.shader->setVec3("lightColor", lightColor);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planet.diffuseTexture);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        particle_shader.use();
        particle_shader.setMat4("projection", projection);
        particle_shader.setMat4("view", view);
        particle_shader.setVec3("cameraRight", glm::vec3(view[0][0], view[1][0], view[2][0]));
        particle_shader.setVec3("cameraUp",    glm::vec3(view[0][1], view[1][1], view[2][1]));

        glBindVertexArray(particles_VAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, alive_particles);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        if (showPanel)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        
            ImGui::Begin("Panel");
            ImGui::Text("EARTH");
            ImGui::SliderFloat("EARTH ORBIT RADIUS: ", &EARTH_ORBIT_RADIUS, 1.0f, 10.0f);
            ImGui::SliderFloat("EARTH SCALE: ", &EARTH_SCALE, 0.0f, 1.0f);

            for (auto& [name, planet] : planets)
            {
                ImGui::Text(planet.name.c_str());
                const std::string orbit_text = planet.name + " ORBIT RADIUS: ";
                const std::string scale_text = planet.name + " SCALE: ";
                const std::string orbit_time_text = planet.name + " ORBIT TIME: ";
                ImGui::SliderFloat(orbit_text.c_str(), &planet.orbitRadius, 0.0f, 100.0f);
                ImGui::SliderFloat(scale_text.c_str(), &planet.scale, 0.0f, 1.0f);
                ImGui::SliderFloat(orbit_time_text.c_str(), &planet.orbitTime, 0.0f, 200.0f);
            }
            ImGui::End();
        
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void buildSphere(int sectorCount, int stackCount, float radius)
{
    vertices.clear();
    indices.clear();
    lineIndices.clear();

    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; i++)
    {
        stackAngle = PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        
        for (int j = 0; j <= sectorCount; j++)
        {
            sectorAngle = j * sectorStep;
            
            x = xy * sinf(sectorAngle);
            y = radius * sinf(stackAngle);;
            z = xy * cosf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);

            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);

            float tx = cosf(sectorAngle);
            float ty = 0.0f;
            float tz = -sinf(sectorAngle);
            vertices.push_back(tx);
            vertices.push_back(ty);
            vertices.push_back(tz);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount-1))
            {
                indices.push_back(k1+1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }

            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if (i != 0)
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }
    std::cout << "===CREATED_A_SPHERE===\n";
    std::cout << "Radius: " << radius << '\n';
    std::cout << "Sector count: " << sectorCount << '\n';
    std::cout << "Stack count: " << stackCount << '\n';
    std::cout << "======================\n";
}

void processInput(GLFWwindow *window)
{
    const float cameraSpeed = 2.5f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraFront * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraFront * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;
    }   
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    if (!showPanel && !showPlanetZoom)
    { 
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
    }
}

unsigned int loadTexture(const char *path)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (data)
    {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        if (nrChannels == 2) format = GL_RG;
        if (nrChannels == 3) format = GL_RGB;
        if (nrChannels == 4) format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "ERROR TEXTURE: failed to load texture from path " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

unsigned int loadCubeMap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "ERROR CUBE MAP TEXTURE: failed to load texture at path " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

void key_callback(GLFWwindow *window, int key, int scancodem, int action, int mods)
{
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        showPanel = !showPanel;

        if (showPanel)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
    
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        showPlanetZoom = !showPlanetZoom;
    }
}
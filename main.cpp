#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "include/shader_m.h"
#include "include/camera.h"
#include "include/tiny_obj_loader.h"
#include "include/stb_image.h"
#include "include/ft2build.h"
#include FT_FREETYPE_H


// Declaração das funções
int loadModel(const std::string& filePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, unsigned int& VAO, unsigned int& VBO, unsigned int& NBO, unsigned int& TBO, bool centerModel);
unsigned int loadCubemap(std::vector<std::string> faces);
void setupSkybox();
void loadLuz();
int loadText();
void renderScene();
bool checkLanding();
bool checkStart();
void animacaoSaida();
void animacaoAterragem(glm::vec3 ponto);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput();
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

// Variáveis globais
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWwindow* window;
Shader* skyboxShader;
Shader* lightingShader;
Shader* lightingCubeShader;
Shader* textShader;

unsigned int CameraMode = 0;

// Lighting
glm::vec3 spotLightPositions[] = {
    glm::vec3(-10.0f, 70.0f, 0.0f),
    glm::vec3(-10.0f, 70.0f, -110.0f),
    glm::vec3(-10.0f, 70.0f, 110.0f),
    glm::vec3(80.0f, 70.0f, 0.0f),
    glm::vec3(80.0f, 70.0f, -110.0f),
    glm::vec3(80.0f, 70.0f, 110.0f),
    glm::vec3(-100.0f, 70.0f, 0.0f),
    glm::vec3(-100.0f, 70.0f, -110.0f),
    glm::vec3(-100.0f, 70.0f, 110.0f)
};
glm::vec3 pointLightPositions[] = {
    glm::vec3(10.0f, 20.0f, 0.0f),
    glm::vec3(1000.0f, 20.0f, 0.0f),
    glm::vec3(500.0f, 60.0f, 100.0f)
};
glm::vec3 starLightColor(0.7f, 0.8f, 1.0f);
glm::vec3 redLightColor(0.8f, 0.0f, 0.0f);
glm::vec3 blueLightColor(0.0f, 0.0f, 0.8f);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Fighter
struct Fighter {
    glm::vec3 position;
    glm::vec3 front;
    float rotation;
    float directionX;
    float directionY;
    float movementSpeed;
        
    Fighter(glm::vec3 pos, glm::vec3 frnt, float rotation, float directionX, float directionY, float speed) 
        : position(pos), front(frnt), rotation(rotation), directionX(directionX), directionY(directionY), movementSpeed(speed) {}
};
Fighter fighter_player(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 10.0f);
std::vector<Fighter> enemies = {
    Fighter(glm::vec3(543.2f, 54.0f, -33.0f), -camera.Front, 0.0f, camera.Yaw, camera.Pitch, 10.0f),
    Fighter(glm::vec3(600.0f, 54.0f, -50.0f), -camera.Front, 0.0f, camera.Yaw, camera.Pitch, 10.0f),
    Fighter(glm::vec3(650.0f, 54.0f, -70.0f), -camera.Front, 0.0f, camera.Yaw, camera.Pitch, 10.0f)
};

// Aterragens
unsigned int estacionado = 1;
std::vector<glm::vec3> pontosAterragem = {
    glm::vec3(453.0f, 54.0f, 33.5f),
    glm::vec3(43.2f, 54.0f, -33.5f)
};

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Vertex data
// VBO (Vertex Buffer Object): Stores vertex data.
// VAO (Vertex Array Object): Stores the configuration of vertex attributes.
// NBO (Normal Buffer Object): Stores normal vector data.
std::vector<glm::vec3> vertices, normals;
unsigned int VBO, VAO, NBO, TBO;
std::vector<glm::vec3> vertices2, normals2;
unsigned int VBO2, VAO2, NBO2, TBO2;
std::vector<glm::vec3> vertices3, normals3;
unsigned int VBO3, VAO3, NBO3, TBO3;
std::vector<glm::vec2> texCoords, texCoords2, texCoords3;
unsigned int lightVBO, lightCubeVAO;
unsigned int skyboxVAO, skyboxVBO;
// Skybox texture
unsigned int cubemapTexture; 
std::vector<std::string> faces = {
    "models/sky/skyboxxa.png",
    "models/sky/skyboxxb.png",
    "models/sky/skyboxya.png",
    "models/sky/skyboxyb.png",
    "models/sky/skyboxza.png",
    "models/sky/skyboxzb.png"
};

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;
unsigned int VAOt, VBOt;

/**
 * @brief A função principal inicializa e configura o GLFW, cria uma janela em fullscreen,
 * usa o GLAD para permitir o uso de todas as funções do OpenGL e é configurado o estado global do mesmo.
 * São compilados e construídos os shaders recorrendo á classe Shader. 
 * Depois carrega os dois modelos (hangar e fighter) e entra no ciclo de renderização.
 * 
 * O ciclo de renderização processa a existência de inputs, verifica condições de aterragem e partida do fighter,
 * renderiza toda a cena. No fim de cada iteração, troca os buffers e pesquisa eventos de IO até que a janela deva fechar.
 * 
 * Após o loop, desaloca recursos e termina o GLFW.
 * 
 * @return int - Retorna 0 se foi bem sucedido, ou -1 se ocorreu algum erro.
 */
int main() {
    // Initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create GLFW window
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "StarWars", primaryMonitor, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Set the input mode to lock the cursor and make it invisible
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Lock the cursor
    glfwSetCursorPos(window, SCR_WIDTH/2, SCR_HEIGHT/2); // Set the initial cursor position

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (loadModel("models/hangar/hangar.obj", vertices, normals, texCoords, VAO, VBO, NBO, TBO, false) == -1)
        return -1;
    if(loadModel("models/tie/tie.obj", vertices2, normals2, texCoords2, VAO2, VBO2, NBO2, TBO2, true) == -1)
        return -1;
    if(loadModel("models/xwing/xwing.obj", vertices3, normals3, texCoords3, VAO3, VBO3, NBO3, TBO3, false) == -1)
        return -1;
    loadLuz();
    loadText();

    // Build and compile shaders
    lightingShader = new Shader("shaders/light.vs", "shaders/light.fs");
    lightingCubeShader = new Shader("shaders/lamp.vs", "shaders/lamp.fs");
    textShader = new Shader("shaders/text.vs", "shaders/text.fs");
    skyboxShader = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    
    // Setup skybox and load texture
    setupSkybox();
    cubemapTexture = loadCubemap(faces);
    
    // Set skybox shader uniforms
    skyboxShader->use();
    skyboxShader->setInt("skybox", 0);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput();
        // Verificar se está perto de um ponto de aterragem
        if(checkLanding());
        // Se não aterrou, então verificar a partida
        else
            checkStart();
        // Render scene
        renderScene();

        const std::string iniciar = "Para comecar aperte ESPACO";
        if(estacionado == 1){
            float textWidth = iniciar.length() * 25.0f; 
            RenderText(iniciar, (SCR_WIDTH - textWidth) / 2.0f, SCR_HEIGHT / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        // RenderText("(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // De-allocate resources
    delete skyboxShader;
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &NBO2);
    glDeleteVertexArrays(1, &VAO3);
    glDeleteBuffers(1, &VBO3);
    glDeleteBuffers(1, &NBO3);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteVertexArrays(1, &VAOt);
    glDeleteBuffers(1, &VBOt);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glfwTerminate();
    return 0;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

/**
 * @brief Função que carrega as varias texturas para o skybox.
 */
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);
    
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        std::cout << "Loading texture: " << faces[i] << std::endl; // Debug print
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                      0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            std::cout << "Successfully loaded texture " << i << std::endl;
        } else {
            std::cout << "Failed to load texture: " << faces[i] << std::endl;
            std::cout << "Error: " << stbi_failure_reason() << std::endl;
            stbi_image_free(data);
            return 0;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// Função que configura o skybox
void setupSkybox() {
    float skyboxVertices[] = {
        // positions          
        -700.0f,  700.0f, -700.0f,
        -700.0f, -700.0f, -700.0f,
         700.0f, -700.0f, -700.0f,
         700.0f, -700.0f, -700.0f,
         700.0f,  700.0f, -700.0f,
        -700.0f,  700.0f, -700.0f,

        -700.0f, -700.0f,  700.0f,
        -700.0f, -700.0f, -700.0f,
        -700.0f,  700.0f, -700.0f,
        -700.0f,  700.0f, -700.0f,
        -700.0f,  700.0f,  700.0f,
        -700.0f, -700.0f,  700.0f,

         700.0f, -700.0f, -700.0f,
         700.0f, -700.0f,  700.0f,
         700.0f,  700.0f,  700.0f,
         700.0f,  700.0f,  700.0f,
         700.0f,  700.0f, -700.0f,
         700.0f, -700.0f, -700.0f,

        -700.0f, -700.0f,  700.0f,
        -700.0f,  700.0f,  700.0f,
         700.0f,  700.0f,  700.0f,
         700.0f,  700.0f,  700.0f,
         700.0f, -700.0f,  700.0f,
        -700.0f, -700.0f,  700.0f,

        -700.0f,  700.0f, -700.0f,
         700.0f,  700.0f, -700.0f,
         700.0f,  700.0f,  700.0f,
         700.0f,  700.0f,  700.0f,
        -700.0f,  700.0f,  700.0f,
        -700.0f,  700.0f, -700.0f,

        -700.0f, -700.0f, -700.0f,
        -700.0f, -700.0f,  700.0f,
         700.0f, -700.0f, -700.0f,
         700.0f, -700.0f, -700.0f,
        -700.0f, -700.0f,  700.0f,
         700.0f, -700.0f,  700.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

/**
 * @brief Função utilizada para carrega um modelo 3D a partir de um ficheiro .obj usando a biblioteca TinyObjLoader 
 * para a leitura do ficheiro. São extraidos os vertices e as normais, e caso necessário centraliza o modelo.
 * Depois configura os objetos de array e buffer de vértices do OpenGL.
 * 
 * @param filePath Caminho para o ficheiro .obj a ser carregado.
 * @param vertices Referência para o vetor onde os dados dos vértices serão guardados.
 * @param normals Referência para o vetor onde os dados das normais serão guardados.
 * @param VAO Referência para o VAO a ser gerado.
 * @param VBO Referência para o VBO a ser gerado.
 * @param NBO Referência para o NBO a ser gerado.
 * @param centerModel Bool indicando se o modelo deve ou não ser centralizado.
 * @return int - Retorna 0 se foi bem sucedido, ou -1 se ocorreu erro ao carregar o ficheiro.
 */
int loadModel(const std::string& filePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, unsigned int& VAO, unsigned int& VBO, unsigned int& NBO, unsigned int& TBO, bool centerModel) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::string baseDir = filePath.substr(0, filePath.find_last_of('/')) + "/";

    // Load the .obj file using TinyObjLoader
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath.c_str(), baseDir.c_str())) {
        std::cout << "Failed to load OBJ file: " << err << std::endl;
        return -1;
    }

    if (!err.empty()) {
        std::cout << "Error: " << err << std::endl;
    }

    std::cout << "Loaded " << shapes.size() << " shapes" << std::endl;
    std::cout << "Loaded " << materials.size() << " materials" << std::endl;

    glm::vec3 center(0.0f);
    int totalVertices = 0;

    // Extract vertices, normals, and texture coordinates for each point in the model
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 vertex(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            );
            vertices.push_back(vertex);
            center += vertex;
            totalVertices++;

            if (!attrib.normals.empty()) {
                glm::vec3 normal(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                );
                normals.push_back(normal);
            }

            if (!attrib.texcoords.empty()) {
                glm::vec2 texCoord(
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                );
                texCoords.push_back(texCoord);
            }
        }
    }

    if (centerModel) {
        center /= totalVertices;
        for (auto& vertex : vertices) {
            vertex -= center;
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &TBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), &texCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(2);

    // Load textures
    auto loadTexture = [&](const std::string& texturePath) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        } else {
            std::cout << "Failed to load texture: " << texturePath << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    };

    for (const auto& material : materials) {
        std::cout << "Material name: " << material.name << std::endl;
        if (!material.diffuse_texname.empty()) {
            std::string texturePath = baseDir + material.diffuse_texname;
            std::cout << "Loading texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }
        if (!material.metallic_texname.empty()) {
            std::string texturePath = baseDir + material.metallic_texname;
            std::cout << "Loading texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }
        if (!material.roughness_texname.empty()) {
            std::string texturePath = baseDir + material.roughness_texname;
            std::cout << "Loading texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }
        if (!material.normal_texname.empty()) {
            std::string texturePath = baseDir + material.normal_texname;
            std::cout << "Loading texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }
    }

    return 0;
}

void loadLuz(){
    float vertices[] = {
        -1.5f, -0.1f, -1.5f, 
         1.5f, -0.1f, -1.5f,  
         1.5f,  0.1f, -1.5f,  
         1.5f,  0.1f, -1.5f,  
        -1.5f,  0.1f, -1.5f, 
        -1.5f, -0.1f, -1.5f, 

        -1.5f, -0.1f,  1.5f, 
         1.5f, -0.1f,  1.5f,  
         1.5f,  0.1f,  1.5f,  
         1.5f,  0.1f,  1.5f,  
        -1.5f,  0.1f,  1.5f, 
        -1.5f, -0.1f,  1.5f, 

        -1.5f,  0.1f,  1.5f, 
        -1.5f,  0.1f, -1.5f, 
        -1.5f, -0.1f, -1.5f, 
        -1.5f, -0.1f, -1.5f, 
        -1.5f, -0.1f,  1.5f, 
        -1.5f,  0.1f,  1.5f, 

         1.5f,  0.1f,  1.5f,  
         1.5f,  0.1f, -1.5f,  
         1.5f, -0.1f, -1.5f,  
         1.5f, -0.1f, -1.5f,  
         1.5f, -0.1f,  1.5f,  
         1.5f,  0.1f,  1.5f,  

        -1.5f, -0.1f, -1.5f, 
         1.5f, -0.1f, -1.5f,  
         1.5f, -0.1f,  1.5f,  
         1.5f, -0.1f,  1.5f,  
        -1.5f, -0.1f,  1.5f, 
        -1.5f, -0.1f, -1.5f, 

        -1.5f,  0.1f, -1.5f, 
         1.5f,  0.1f, -1.5f,  
         1.5f,  0.1f,  1.5f,  
         1.5f,  0.1f,  1.5f,  
        -1.5f,  0.1f,  1.5f, 
        -1.5f,  0.1f, -1.5f, 
    };

    // Remove the re-declaration of lightCubeVAO
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

int loadText(){
    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

	// find path to font
    std::string font_name = "fonts/ARIAL.TTF";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
	
	// load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    
    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAOt);
    glGenBuffers(1, &VBOt);
    glBindVertexArray(VAOt);
    glBindBuffer(GL_ARRAY_BUFFER, VBOt);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return 0;
}

/**
 * @brief Esta função itera pela lista de pontos de aterragem e verifica se o fighter a menos de uma certa distância de qualquer um desses pontos. 
 * Se o fighter estiver dentro da distância e a tecla de espaço for pressionada, é acionada a animação de aterragem e ajustada a orientação da câmera e do fighter.
 *
 * @param lightingShader Referência ao shader usado para efeitos de iluminação.
 * @return bool - true se tiver sido realizada uma aterragem, ou false caso contrário.
 */
bool checkLanding() {
    for (glm::vec3 ponto : pontosAterragem) {
        float distance = glm::distance(fighter_player.position, ponto);
        if (estacionado == 0 && distance < 20.0f) {
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                animacaoAterragem(ponto);
                camera.Front.x = -camera.Front.x;
                fighter_player.front.x = camera.Front.x;
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Esta função verifica se o fighter está num estado de "estacionado" e caso esteja e a tecla de espaço for pressionada, é acionada a animação de partida.
 *
 * @param lightingShader Referência ao shader usado para efeitos de iluminação.
 * @return bool - true se tiver sido realizada a partida, ou false caso contrário.
 */
bool checkStart() {
    if(estacionado != 0){
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            animacaoSaida();
            return true;
        }
    }
    return false;
}

/**
 * @brief Esta função anima a posição vertical do fighter da sua posição atual para uma altura de 20.0 unidades, esta animação dura 2 segundos. 
 * Durante a animação, a cena é continuamente renderizada e os buffers da janela são trocados para exibir a posição atualizada.
 * 
 * @param lightingShader Referência ao shader usado para iluminação na cena.
 */
void animacaoSaida(){
    float startY = fighter_player.position.y;
    float endY = 20.0f;
    float duration = 2.0f; // duration in seconds
    float startTime = glfwGetTime();
    
    while (glfwGetTime() - startTime < duration) {
        float currentTime = glfwGetTime();
        float t = (currentTime - startTime) / duration;
        fighter_player.position.y = glm::mix(startY, endY, t);
        
        // Render scene
        renderScene();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    fighter_player.position.y = endY;

    estacionado = 0;
}

/**
 * @brief Esta função anima a aterragem de um fighter, ajustando sua direção, rotação e posição ao longo do tempo, 
 * de forma a regressar a uma posição neutra e virado para a saída do hangar em que aterrou. 
 * 
 * @param lightingShader Referência ao shader usado para iluminação na cena.
 * @param ponto O ponto de aterragem alvo.
 */
void animacaoAterragem(glm::vec3 ponto) {
    estacionado = 1;

    float endDirectionY = 0.0f;
    float endRotation = 0.0f;
    // Primeiramente ajustar a direção Y e rotação de modo a que o fighter não esteja inclinado em nenhuma direção
    while (fabs(fighter_player.directionY - endDirectionY) > 0.1f || fabs(fighter_player.rotation - endRotation) > 0.1f) {
        float t = deltaTime * 1.0f;
        fighter_player.directionY = glm::mix(fighter_player.directionY, endDirectionY, t);
        fighter_player.rotation = glm::mix(fighter_player.rotation, endRotation, t);
       
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    float endDirectionX = 180.0f;
    // Ajustar a direção X do fighter para a saída do hangar caso necessário
    if (fighter_player.directionX > 100 && fighter_player.directionX < 260) {
        endDirectionX = 0.0f;
    }
    // Depois rodar o fighter de forma a que fique virado para a saida o hnagar
    while (fabs(fighter_player.directionX - endDirectionX) > 0.5f) {
        float t = deltaTime * 0.5f;
        fighter_player.directionX = glm::mix(fighter_player.directionX, endDirectionX, t);
      
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Por fim mover o fighter para a posição desejada
    while (glm::distance(fighter_player.position, ponto) > 0.1f) {
        float t = deltaTime * 1.0f;
        fighter_player.position = glm::mix(fighter_player.position, ponto, t);

        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ajusta os ângulos da camera no final da animção
    camera.Yaw = endDirectionX;
    camera.Pitch = 0.0f;
}


/**
 * @brief Esta função verifica inputs de teclas específicas e atualiza a posição e rotação do fighter com base no input. 
 * Também lida com o evento de fecho da janela quando a tecla ESC é pressionada.
 * A função também verifica se o fighter está estacionado, caso esteja, nenhum movimento ou rotação é processado.
 * 
 * Correspondência das teclas:
 * - ESC: Fecha a janela.
 * - W: Move o fighter para frente.
 * - S: Move o fighter para trás.
 * - A: Inclina o fighter para a esquerda.
 * - D: Inclina o fighter para a direita.
 */
void processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(estacionado != 0)
        return;

    // Toggle camera modes (free camera vs. fixed behind the player)
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        CameraMode = 1 - CameraMode;
    }

    static float lastPressTime = 0.0f;

    // Move forward with smooth acceleration
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (fighter_player.movementSpeed < 100.0f) {
            fighter_player.movementSpeed += 50.0f;
        }
        lastPressTime = glfwGetTime();
    }
    // Move backward with smooth deceleration
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (fighter_player.movementSpeed > 20.0f) {
            fighter_player.movementSpeed -= 50.0f * deltaTime;
        }
    }
    else {
        // Gradual deceleration
        float currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 1.0f) { // 1 second delay before deceleration
            if (fighter_player.movementSpeed > 10.0f) {
                fighter_player.movementSpeed -= 20.0f * deltaTime; // Decelerate more gradually
            } else {
                fighter_player.movementSpeed = 20.0f; // Minimum speed
            }
        }
    }
}

/**
 * @brief Esta função é chamada sempre que o rato é movido, de modo a atualizar a direção da câmera com base nesse movimento 
 * e ajusta também a direção e o vetor frontal do fighter de acordo com a câmera.
 * 
 * @param window Referência para a janela GLFW que recebeu o evento.
 * @param xpos A nova coordenada x do cursor do rato.
 * @param ypos A nova coordenada y do cursor do rato.
 */
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(estacionado == 1)
        return;

    // if (firstMouse)
    // {
    //     lastX = xpos;
    //     lastY = ypos;
    //     firstMouse = false;
    // }
    
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float xoffset = xpos - (width/2);
    float yoffset = (height/2) - ypos;

    // float xoffset = xpos - lastX;
    // float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    // lastX = xpos;
    // lastY = ypos;

    // printf("x: %f\n", xoffset);
    // printf("y: %f\n", yoffset);

    float sensitivity = 0.01f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update the camera's direction based on mouse input
    camera.ProcessMouseMovement(xoffset, yoffset);

    // Update the direction of the fighter_player based on the camera
    fighter_player.front = camera.Front;
    fighter_player.directionX = camera.Yaw;
    fighter_player.directionY = camera.Pitch;

    // Smoothly rotate the fighter based on the camera's yaw
    const float rotationSpeed = 5.0f;
    fighter_player.rotation -= xoffset * rotationSpeed * deltaTime;

    // Re-center the cursor in the middle of the window
    glfwSetCursorPos(window, width / 2.0, height / 2.0);
}


/**
 * @brief Esta função é chamada sempre que um evento de scroll é detectado na janela GLFW, para ajustar o nível de zoom da câmera.
 * 
 * @param window Referência para a janela GLFW que recebeu o evento.
 * @param xoffset O deslocamento de scroll ao longo do eixo x.
 * @param yoffset O deslocamento de scroll ao longo do eixo y, usado para ajustar o nível de zoom da câmera.
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

/**
 * @brief Esta função renderiza a cena com iluminação e os objetos.
 * Começa por limpar a tela, configurar a posição da câmera e configurar o shader de iluminação. Depois renderiza os dois hangares e o fighter 
 * com a respetiva configuração de iluminação. 
 * Nesta função é também determinada a fonte de luz mais próxima do fighter e ajusta a sua iluminação de acordo.
 * 
 * @param lightingShader Referência ao programa de shader usado para iluminação.
 */
void renderScene() {
    // Clear the screen
    glClearColor(0.01f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Update camera position
    if(CameraMode == 0){
        glm::vec3 cameraOffset(0.0f, 10.0f, 60.0f);
        camera.Position = glm::mix(camera.Position, fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, cameraOffset.y, 0.0f), 0.1f);
    }
    else {
        camera.Position = fighter_player.position;
    }

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    lightingShader->use();
    // lightingShader.setInt("material.diffuse", 0);
    // lightingShader.setInt("material.specular", 1);
    lightingShader->setMat4("projection", projection);
    lightingShader->setMat4("view", view);
    lightingShader->setVec3("viewPos", camera.Position);

    // directional light
    lightingShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader->setVec3("dirLight.ambient", starLightColor * glm::vec3(0.05f));
    lightingShader->setVec3("dirLight.diffuse", starLightColor * glm::vec3(0.5f));
    lightingShader->setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);

    // point light 1
    lightingShader->setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader->setVec3("pointLights[0].ambient", blueLightColor * glm::vec3(0.3f));
    lightingShader->setVec3("pointLights[0].diffuse", blueLightColor * glm::vec3(1.0f));
    lightingShader->setVec3("pointLights[0].specular", 0.6f, 0.6f, 0.6f);
    lightingShader->setFloat("pointLights[0].constant", 1.0f);
    lightingShader->setFloat("pointLights[0].linear", 0.002f);
    lightingShader->setFloat("pointLights[0].quadratic", 0.0001f);
    // point light 2
    lightingShader->setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader->setVec3("pointLights[1].ambient", redLightColor * glm::vec3(0.3f));
    lightingShader->setVec3("pointLights[1].diffuse", redLightColor * glm::vec3(1.0f));
    lightingShader->setVec3("pointLights[1].specular", 0.6f, 0.6f, 0.6f);
    lightingShader->setFloat("pointLights[1].constant", 1.0f);
    lightingShader->setFloat("pointLights[1].linear", 0.002f);
    lightingShader->setFloat("pointLights[1].quadratic", 0.0001f);
    // point light 3
    lightingShader->setVec3("pointLights[2].position", pointLightPositions[2]);
    lightingShader->setVec3("pointLights[2].ambient", blueLightColor * glm::vec3(0.5f));
    lightingShader->setVec3("pointLights[2].diffuse", blueLightColor * glm::vec3(1.5f));
    lightingShader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("pointLights[2].constant", 1.0f);
    lightingShader->setFloat("pointLights[2].linear", 0.001f);
    lightingShader->setFloat("pointLights[2].quadratic", 0.00005f);

    // spot lights
    int i = 0;
    for (glm::vec3 pos : spotLightPositions) {
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].position", pos);
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(25.0f)));
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(40.0f)));
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].ambient", 0.4f, 0.4f, 0.4f);
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0f);
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].linear", 0.014f);
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.0007f);
        i++;
    }
    // spot lights do segundo hangar
    i = 9;
    for (glm::vec3 pos : spotLightPositions) {
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].position", (pos + glm::vec3(1020.0f, 0.0f, 0.0f)));
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(25.0f)));
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(40.0f)));
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].ambient", 0.4f, 0.4f, 0.4f);
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader->setVec3("spotLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0f);
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].linear", 0.014f);
        lightingShader->setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.0007f);
        i++;
    }


    // Render hangars
    auto renderHangar = [&](glm::vec3 translation, float rotation) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader->setVec3("material.ambient", 0.3f, 0.3f, 0.3f);
        lightingShader->setVec3("material.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader->setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        lightingShader->setFloat("material.shininess", 25.0f);
        lightingShader->setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    };
    renderHangar(glm::vec3(0.0f, 0.0f, 0.0f), 0.0f);
    renderHangar(glm::vec3(1000.0f, 0.0f, 0.0f), 180.0f);

    // Render fighter
    glm::mat4 model = glm::translate(glm::mat4(1.0f), fighter_player.position);
    model = glm::rotate(model, glm::radians(fighter_player.directionX + 270), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    model = glm::scale(model, glm::vec3(35.0f));
    lightingShader->setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
    lightingShader->setVec3("material.diffuse", 0.3f, 0.3f, 0.3f);
    lightingShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    lightingShader->setFloat("material.shininess", 32.0f);
    lightingShader->setMat4("model", model);
    glBindVertexArray(VAO2);
    glDrawArrays(GL_TRIANGLES, 0, vertices2.size());

    // Render inimigos
    for(Fighter enemy : enemies){
        model = glm::translate(glm::mat4(1.0f), enemy.position);
        model = glm::rotate(model, glm::radians(enemy.directionX + 270), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(3.0f));
        lightingShader->setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader->setVec3("material.diffuse", 0.3f, 0.3f, 0.3f);
        lightingShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        lightingShader->setFloat("material.shininess", 32.0f);
        lightingShader->setMat4("model", model);
        glBindVertexArray(VAO3);
        glDrawArrays(GL_TRIANGLES, 0, vertices3.size());
    }

    // also draw the lamp object
    lightingCubeShader->use();
    lightingCubeShader->setMat4("projection", projection);
    lightingCubeShader->setMat4("view", view);
    for(glm::vec3 pos : spotLightPositions){
        model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::scale(model, glm::vec3(2.0f));
        lightingCubeShader->setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    for(glm::vec3 pos : spotLightPositions){
        model = glm::translate(glm::mat4(1.0f), (pos + glm::vec3(1020.0f, 0.0f, 0.0f)));
        model = glm::scale(model, glm::vec3(2.0f));
        lightingCubeShader->setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    //movimento automatico em direção ao hangar inimigo
    if(estacionado == 0){
        glm::vec3 direction = glm::vec3(
            cos(glm::radians(fighter_player.directionY)) * cos(glm::radians(fighter_player.directionX)),
            sin(glm::radians(fighter_player.directionY)),
            cos(glm::radians(fighter_player.directionY)) * sin(glm::radians(fighter_player.directionX))
        );
        fighter_player.position += fighter_player.movementSpeed * direction * deltaTime;
    }

    // Then render skybox
    glDepthFunc(GL_LEQUAL);  // Change depth function
    skyboxShader->use();
    view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation
    skyboxShader->setMat4("view", view);
    skyboxShader->setMat4("projection", projection);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthFunc(GL_LESS); // Restore depth function
}

void RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader->use();
    textShader->setMat4("projection", projection);
    glUniform3f(glGetUniformLocation(textShader->ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0); 
    glBindVertexArray(VAOt);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // printf("%d\n", ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBOt);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
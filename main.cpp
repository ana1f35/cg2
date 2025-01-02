#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
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


// Fighter
struct Fighter {
    glm::vec3 position;
    glm::vec3 front;
    float rotation;
    float directionX;
    float directionY;
    float movementSpeed;
    int hp;
    float collisionRadius;
        
    Fighter(glm::vec3 pos, glm::vec3 frnt, float rotation, float directionX, float directionY, float speed, int hp, float radius) 
        : position(pos), front(frnt), rotation(rotation), directionX(directionX), directionY(directionY), movementSpeed(speed), hp(hp), collisionRadius(radius) {}
};

struct Projectile {
    glm::vec3 position;
    glm::vec3 direction;
    float speed;
    float collisionRadius;

    Projectile(glm::vec3 pos, glm::vec3 dir, float spd, float radius)
        : position(pos), direction(glm::normalize(dir)), speed(spd), collisionRadius(radius) {}
};

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

// Declaração das funções
int loadModel(const std::string& filePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, unsigned int& VAO, unsigned int& VBO, unsigned int& NBO, unsigned int& TBO, bool centerModel);
unsigned int loadCubemap(std::vector<std::string> faces);
void setupSkybox();
void loadLuz();
int loadText();
void renderScene();
bool checkStart();
void animacaoSaida();
void moverInimigos();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput();
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, bool useFirstFont);
bool isColliding(const glm::vec3& pos1, float radius1, const glm::vec3& pos2, float radius2);
void checkCollisions();
void renderBoundingBox(glm::vec3 position, float radius, float scaleFactor);
void shootProjectile(const Fighter& fighter);
void updateProjectiles(float deltaTime);
void setupBoundingBox();
void restartGame();

// Variáveis globais
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWwindow* window;
Shader* skyboxShader;
Shader* lightingShader;
Shader* lightingCubeShader;
Shader* textShader;
Shader* hitBoxShader;

unsigned int cameraMode = 0;
unsigned int gameState = 0;
int pontuacao = 0;

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

glm::vec3 hangarPos(0.0f, 0.0f, 0.0f);
glm::vec3 enemyHangarPos(1000.0f, 0.0f, 0.0f);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Fighter fighter_player(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 30.0f, 3, 10.0f);
// Inicialmente estão 3 estacionados
std::vector<Fighter> enemies = {
    Fighter(glm::vec3(1000.0f, 5.0f, -80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 15.0f, 1, 10.0f),
    Fighter(glm::vec3(900.0f, 5.0f, 0.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 15.0f, 1, 10.0f),
    Fighter(glm::vec3(1000.0f, 5.0f, 80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 15.0f, 1, 10.0f)
};

std::vector<Projectile> projectiles;

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
unsigned int boundingBoxVBO, boundingBoxEBO, boundingBoxVAO;

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

std::map<GLchar, Character> Characters, Characters2;
unsigned int VAOt, VBOt;
glm::vec3 textColor = glm::vec3(255.0f / 255.0f, 232.0f / 255.0f, 31.0f / 255.0f);

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
    hitBoxShader = new Shader("shaders/hitbox.vs", "shaders/hitbox.fs");
    
    // Setup skybox and load texture
    setupSkybox();
    setupBoundingBox();
    cubemapTexture = loadCubemap(faces);
    
    // Set skybox shader uniforms
    skyboxShader->use();
    skyboxShader->setInt("skybox", 0);

    glm::vec3 cameraOffset(0.0f, 10.0f, 60.0f);
    camera.Position = fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, cameraOffset.y, 0.0f);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput();
        renderScene();
        if(gameState == 0){
            const std::string titulo = "- War of StArs -";
            const std::string iniciar = "Press SPACE To Start";
            float textWidth2 = titulo.length() * 42.0f; 
            RenderText(titulo, (SCR_WIDTH - textWidth2) / 2.0f, SCR_HEIGHT / 2.0f + 50.0f, 1.0f, textColor, false);
            float textWidth = iniciar.length() * 25.0f; 
            RenderText(iniciar, (SCR_WIDTH - textWidth) / 2.0f, SCR_HEIGHT / 2.0f - 50.0f, 1.0f, textColor, true);
            checkStart();
        }
        // Durante o jogo mostra: vida e pontuação
        else if(gameState == 1){
            std::string vida = "Health: " + std::to_string(fighter_player.hp);
            std::string pontuacaoStr = "Points: " + std::to_string(pontuacao);
            RenderText(vida, SCR_WIDTH - 300.0f, SCR_HEIGHT - 100.0f, 1.0f, textColor, true);
            RenderText(pontuacaoStr, SCR_WIDTH - 300.0f, SCR_HEIGHT - 180.0f, 1.0f, textColor, true);
            moverInimigos();

            // o jogo termina se o player ficou sem vida, ou se já chegou ao hangar inimigo
            if(fighter_player.position.x >= enemyHangarPos.x - 100)
                gameState = 4;
            else if (fighter_player.hp == 0)
                gameState = 4;
        }
        // Em pausa (controlos)
        else if(gameState == 2){
            const std::string controlsTitle = "Game Controls";
            const std::string controlsText1 = "W - Increase Speed";
            const std::string controlsText2 = "S - Decrease Speed";
            const std::string controlsText3 = "V - Toggle Camera View";
            const std::string controlsText6 = "ESC - Exit Game";
            const std::string controlsText7 = "C - Close Controls Menu";

            float titleWidth = controlsTitle.length() * 48.0f;
            RenderText(controlsTitle, (SCR_WIDTH - titleWidth) / 2.0f, SCR_HEIGHT / 2.0f + 380.0f, 1.0f, textColor, false);
            float textWidth1 = controlsText1.length() * 25.0f;
            RenderText(controlsText1, (SCR_WIDTH - textWidth1) / 2.0f, SCR_HEIGHT / 2.0f + 160.0f, 1.0f, textColor, true);
            float textWidth2 = controlsText2.length() * 25.0f;
            RenderText(controlsText2, (SCR_WIDTH - textWidth2) / 2.0f, SCR_HEIGHT / 2.0f + 80.0f, 1.0f, textColor, true);
            float textWidth3 = controlsText3.length() * 25.0f;
            RenderText(controlsText3, (SCR_WIDTH - textWidth3) / 2.0f, SCR_HEIGHT / 2.0f, 1.0f, textColor, true);
            float textWidth6 = controlsText6.length() * 25.0f;
            RenderText(controlsText6, (SCR_WIDTH - textWidth6) / 2.0f, SCR_HEIGHT / 2.0f - 150.0f, 1.0f,  textColor, true);
            float textWidth7 = controlsText7.length() * 25.0f;
            RenderText(controlsText7, (SCR_WIDTH - textWidth7) / 2.0f, SCR_HEIGHT / 2.0f - 250.0f, 1.0f, textColor, true);
        }
        // Terminado
        else if(gameState == 4){
            std::string gameOver = "Game over";
            std::string finalScore = "Final Score: " + std::to_string(pontuacao);
            std::string finalHealth = "HP Left: " + std::to_string(fighter_player.hp);
            std::string restart = "Press SPACE To Restart";
            float gameOverWidth = gameOver.length() * 48.0f;
            RenderText(gameOver, (SCR_WIDTH - gameOverWidth) / 2.0f, SCR_HEIGHT / 2.0f + 100.0f, 1.0f, textColor, false);
            float scoreWidth = finalScore.length() * 25.0f;
            RenderText(finalScore, (SCR_WIDTH - scoreWidth) / 2.0f, SCR_HEIGHT / 2.0f, 1.0f, textColor, true);
            float healthWidth = finalHealth.length() * 25.0f;
            RenderText(finalHealth, (SCR_WIDTH - healthWidth) / 2.0f, SCR_HEIGHT / 2.0f - 100.0f, 1.0f, textColor, true);
            float restartWidth = restart.length() * 25.0f;
            RenderText(restart, (SCR_WIDTH - restartWidth) / 2.0f, SCR_HEIGHT / 2.0f - 200.0f, 1.0f, textColor, true);
        }

        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;

        updateProjectiles(deltaTime);
        checkCollisions();

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

    // find paths to fonts
    std::string font_name1 = "fonts/News Gothic Bold.ttf";
    std::string font_name2 = "fonts/Starjedi.ttf";
    if (font_name1.empty() || font_name2.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // Load first font
    FT_Face face1;
    if (FT_New_Face(ft, font_name1.c_str(), 0, &face1)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face1, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face1, c, FT_LOAD_RENDER))
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
                face1->glyph->bitmap.width,
                face1->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face1->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face1->glyph->bitmap.width, face1->glyph->bitmap.rows),
                glm::ivec2(face1->glyph->bitmap_left, face1->glyph->bitmap_top),
                static_cast<unsigned int>(face1->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face1);

    // Load second font
    FT_Face face2;
    if (FT_New_Face(ft, font_name2.c_str(), 0, &face2)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face2, 0, 72);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face2, c, FT_LOAD_RENDER))
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
                face2->glyph->bitmap.width,
                face2->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face2->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face2->glyph->bitmap.width, face2->glyph->bitmap.rows),
                glm::ivec2(face2->glyph->bitmap_left, face2->glyph->bitmap_top),
                static_cast<unsigned int>(face2->glyph->advance.x)
            };
            Characters2.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face2);

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
 * @brief Esta função verifica se o fighter está num estado de "estacionado" e caso esteja e a tecla de espaço for pressionada, é acionada a animação de partida.
 *
 * @param lightingShader Referência ao shader usado para efeitos de iluminação.
 * @return bool - true se tiver sido realizada a partida, ou false caso contrário.
 */
bool checkStart() {
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        gameState = 3;
        animacaoSaida();
        gameState = 1;
        return true;
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
}

void moverInimigos() {
    static float lastTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    for (auto it = enemies.begin(); it != enemies.end(); ) {
        // mover em direção ao player
        glm::vec3 direction = glm::normalize(fighter_player.position - it->position);
        glm::vec3 newPosition = it->position + direction * it->movementSpeed * deltaTime;

        // Distância minima ao player
        if (glm::distance(newPosition, fighter_player.position) >= 50.0f) {
            it->position = newPosition;
        }

        // TODO: Verificar se o inimigo está fora do ângulo de visão do player
        // float angleToPlayer = glm::degrees(atan2(direction.z, direction.x)) - fighter_player.directionX;
        // if (angleToPlayer < -180.0f) angleToPlayer += 360.0f;
        // if (angleToPlayer > 180.0f) angleToPlayer -= 360.0f;
        // if (angleToPlayer < -50.0f || angleToPlayer > 50.0f) {
        //     // Se o inimigo está fora do ângulo de visão, removê-lo
        //     it = enemies.erase(it);
        // } else {
        //     ++it;
        // }

        // rodar na direção correta
        float targetYaw = glm::degrees(atan2(direction.z, direction.x)) + 90.0f;
        float targetPitch = - glm::degrees(asin(direction.y));
        it->directionX = glm::mix(it->directionX, targetYaw, deltaTime * 2.0f);
        it->directionY = glm::mix(it->directionY, targetPitch, deltaTime * 2.0f);
        it->front = glm::vec3(
            cos(glm::radians(it->directionY)) * cos(glm::radians(it->directionX)),
            sin(glm::radians(it->directionY)),
            cos(glm::radians(it->directionY)) * sin(glm::radians(it->directionX))
        );

        ++it;
    }

    // distância minima entre os inimigos
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (glm::distance(enemies[i].position, enemies[j].position) < 100.0f) {
                glm::vec3 direction = glm::normalize(enemies[j].position - enemies[i].position);
                enemies[j].position = enemies[i].position + direction * 100.0f;
            }
        }
    }
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

    if(gameState == 0 || gameState == 3)
        return;

    if(gameState == 4){
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            restartGame();
            gameState = 1;
        }
        return;
    }

    static bool cKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cKeyPressed) {
            if (gameState == 1)
                gameState = 2;
            else if (gameState == 2)
                gameState = 1;
            cKeyPressed = true;
        }
    } else {
        cKeyPressed = false;
    }

    if(gameState == 2)
        return;

    // Toggle camera modes (free camera vs. fixed behind the player)
    static bool vKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!vKeyPressed) {
            if(cameraMode == 0)
                cameraMode = 1;
            else
                cameraMode = 0;
            vKeyPressed = true;
        }
    } else {
        vKeyPressed = false;
    }

    static float lastPressTime = 0.0f;

    // Move forward with smooth acceleration
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (fighter_player.movementSpeed < 50.0f) {
            fighter_player.movementSpeed += 50.0f;
        }
        lastPressTime = glfwGetTime();
    }
    // Move backward with smooth deceleration
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (fighter_player.movementSpeed > 30.0f) {
            fighter_player.movementSpeed -= 5.0f;
        } else {
            fighter_player.movementSpeed = 30.0f; // Minimum speed
        }
    }
    else {
        // Gradual deceleration
        float currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 1.0f) { // 1 second delay before deceleration
            if (fighter_player.movementSpeed > 30.0f) {
                fighter_player.movementSpeed -= 5.0f; // Decelerate more gradually
            } else {
                fighter_player.movementSpeed = 30.0f; // Minimum speed
            }
        }
    }

    static bool fireKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (!fireKeyPressed) {
            shootProjectile(fighter_player);
            fireKeyPressed = true;
        }
        } else {
            fireKeyPressed = false;
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
    if(gameState != 1)
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
    if (camera.Yaw > 50.0f)
        camera.Yaw = 50.0f;
    if (camera.Yaw < -50.0f)
        camera.Yaw = -50.0f;

    // Update the direction of the fighter_player based on the camera
    fighter_player.front = camera.Front;
    fighter_player.directionX = camera.Yaw;
    fighter_player.directionY = camera.Pitch;

    // Smoothly rotate the fighter based on the camera's yaw
    const float rotationSpeed = 5.0f;
    fighter_player.rotation -= xoffset * rotationSpeed * 0.05;
    if (fighter_player.rotation > 45.0f) {
        fighter_player.rotation = 45.0f;
    } else if (fighter_player.rotation < -45.0f) {
        fighter_player.rotation = -45.0f;
    }

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
    
    //movimento automatico em direção ao hangar inimigo
    if(gameState == 1){
        glm::vec3 direction = glm::vec3(
            cos(glm::radians(fighter_player.directionY)) * cos(glm::radians(fighter_player.directionX)),
            sin(glm::radians(fighter_player.directionY)),
            cos(glm::radians(fighter_player.directionY)) * sin(glm::radians(fighter_player.directionX))
        );
        fighter_player.position += fighter_player.movementSpeed * direction * 0.05f;
    }

    // Update camera position
    if(cameraMode == 0){
        glm::vec3 cameraOffset(0.0f, 10.0f, 60.0f);
        glm::vec3 finalPos = glm::vec3(fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, cameraOffset.y, 0.0f));
        if (glm::distance(camera.Position, finalPos) < 5)
            camera.Position = glm::mix(camera.Position, finalPos, 0.5f);
        else
            camera.Position = glm::mix(camera.Position, finalPos, 0.1f);
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
        lightingShader->setVec3("material.specular", 0.6f, 0.6f, 0.6f);
        lightingShader->setFloat("material.shininess", 25.0f);
        lightingShader->setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    };
    renderHangar(hangarPos, 0.0f);
    renderHangar(enemyHangarPos, 180.0f);

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
        model = glm::rotate(model, glm::radians(enemy.directionX), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(4.0f));
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

    glm::vec3 position = fighter_player.position;
    float radius = fighter_player.collisionRadius;

    renderBoundingBox(position, radius, 1.0);

    // Renderizar bounding boxes para os inimigos
    for (const auto& enemy : enemies) {
        renderBoundingBox(enemy.position, enemy.collisionRadius, 1.2);
    }

}

void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, bool useFirstFont = true)
{
    // activate corresponding render state	
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader->use();
    textShader->setMat4("projection", projection);
    glUniform3f(glGetUniformLocation(textShader->ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0); 
    glBindVertexArray(VAOt);

    // Choose the font to use
    std::map<GLchar, Character>& fontCharacters = useFirstFont ? Characters : Characters2; // Modify this line to use the second font's Characters map

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = fontCharacters[*c];

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

bool isColliding(const glm::vec3& pos1, float radius1, const glm::vec3& pos2, float radius2) {
    float distance = glm::distance(pos1, pos2);
    return distance < (radius1 + radius2);
}

void checkCollisions() {
for (auto& enemy : enemies) {
        if (enemy.hp <= 0) continue; // Ignore inimigos já destruídos

        for (auto& proj : projectiles) {
            if (isColliding(proj.position, proj.collisionRadius, enemy.position, enemy.collisionRadius)) {
                // Colisão detectada
                enemy.hp = 0; // Marca o inimigo como destruído
                proj.position.z = 10000.0f; // Move o projétil para fora
                pontuacao += 10; // Incrementa a pontuação
                break;
            }
        }
    }

    // Remover projéteis fora da tela
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& proj) {
        return proj.position.z > 9999.0f;
    }), projectiles.end());
}

void renderBoundingBox(glm::vec3 position, float radius, float scaleFactor) {
    std::cout << "Rendering bounding box at position: " 
              << position.x << ", " << position.y << ", " << position.z 
              << " with radius: " << radius << std::endl;

    glDisable(GL_DEPTH_TEST);  // Disable depth testing
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Wire frame mode

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(radius * scaleFactor));

    hitBoxShader->use();
    hitBoxShader->setMat4("model", model);
    hitBoxShader->setMat4("view", camera.GetViewMatrix());
    hitBoxShader->setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f));
    hitBoxShader->setVec3("boxColor", glm::vec3(1.0f, 0.0f, 0.0f));

    glBindVertexArray(boundingBoxVAO);
    glLineWidth(2.0f);  // Make lines thicker
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Restore fill mode
    glEnable(GL_DEPTH_TEST);  // Re-enable depth testing
}

void shootProjectile(const Fighter& fighter) {
    // Define a posição inicial e a direção do projétil
    glm::vec3 projectilePosition = fighter.position + fighter.front * 5.0f; // Ajuste o offset inicial
    glm::vec3 projectileDirection = fighter.front;
    float projectileSpeed = 100.0f; // Velocidade do projétil
    float collisionRadius = 2.0f;  // Raio de colisão

    projectiles.emplace_back(projectilePosition, projectileDirection, projectileSpeed, collisionRadius);
}

void updateProjectiles(float deltaTime) {
    for (auto& proj : projectiles) {
        proj.position += proj.direction * proj.speed * deltaTime;
    }

    // Remover projéteis que saíram dos limites
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& proj) {
        return proj.position.z > 1000.0f || proj.position.z < -1000.0f; // Limites arbitrários
    }), projectiles.end());
}

void setupBoundingBox() {
    // Defina os vértices da bounding box
    float boundingBoxVertices[] = {
        // Define the vertices for the bounding box (a cube)
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
    };

    // Defina os índices para desenhar as linhas da bounding box
    unsigned int boundingBoxIndices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    glGenVertexArrays(1, &boundingBoxVAO);
    glGenBuffers(1, &boundingBoxVBO);
    glGenBuffers(1, &boundingBoxEBO);

    glBindVertexArray(boundingBoxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, boundingBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundingBoxVertices), boundingBoxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundingBoxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boundingBoxIndices), boundingBoxIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void restartGame(){
    cameraMode = 0;
    pontuacao = 0;
    // Inicialmente estão 3 estacionados
    enemies = {
        Fighter(glm::vec3(1000.0f, 5.0f, -80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 15.0f, 1, 10.0f),
        Fighter(glm::vec3(900.0f, 5.0f, 0.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 15.0f, 1, 10.0f),
        Fighter(glm::vec3(1000.0f, 5.0f, 80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 15.0f, 1, 10.0f)
    };

    camera = glm::vec3(0.0f, 0.0f, 0.0f);
    fighter_player = Fighter(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 30.0f, 3, 10.0f);

    animacaoSaida();
}
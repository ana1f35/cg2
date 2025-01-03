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
#include "audio.cpp"

struct MaterialInfo {
    std::string name;
    std::string diffuse_texname;
    std::string specular_texname;
    std::string normal_texname;
    unsigned int diffuse_texid;
    unsigned int specular_texid;
    unsigned int normal_texid;
};

struct Model {
    std::string name;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    unsigned int VAO, VBO, NBO, TBO;
    std::vector<MaterialInfo> materials;
};

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
    int origin; // 0 = player, 1 = enemy

    Projectile(glm::vec3 pos, glm::vec3 dir, float spd, float radius, int origin)
        : position(pos), direction(glm::normalize(dir)), speed(spd), collisionRadius(radius), origin(origin) {}
};

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

// Declaração das funções
int loadModel(const std::string& filePath, Model& model, bool centerModel);
void loadMaterials(const std::string& mtlFilePath, std::vector<MaterialInfo>& materials);
unsigned int loadTexture(const char* path);
void renderTextures(Model *model);
void lightsActivate(Shader *ls, glm::mat4 view, glm::mat4 projection);
unsigned int loadCubemap(std::vector<std::string> faces);
void setupSkybox();
void loadLuz();
int loadText();
void renderScene();
bool checkStart();
void animacaoSaida();
void moverInimigos();
void animacaoInimigos();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput();
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, bool useFirstFont);
bool isColliding(glm::vec3 projPosition, float projRadius, glm::vec3 enemyPosition, float enemyRadius);
void checkCollisions();
void renderBoundingBox(glm::vec3 position, float radius, float scaleFactor);
void renderProjectiles(Shader& shader);
void shootProjectile(const Fighter& fighter);
void updateProjectiles(float deltaTime);
void shootEnemyProjectiles(float deltaTime);
void setupBoundingBox();
void createShot(float radius, float height, int segments, unsigned int& VAO, unsigned int& VBO);
Fighter* findClosestEnemy(const glm::vec3& playerPosition);
void desenhaAlvo();
void restartGame();
void renderIntro();

// Variáveis globais
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWwindow* window;
std::map<std::string, std::vector<MaterialInfo>> modelMaterials;

Model hangarModel;
Model tieModel;
Model xwingModel;

Shader* skyboxShader;
Shader* lightingShader;
Shader* lightingCubeShader;
Shader* textShader;
Shader* hitBoxShader;
Shader* particleShader;
Shader* lightingPTexShader;

unsigned int cameraMode = 0;
unsigned int gameState = 5;
int pontuacao = 0;

glm::vec3 hangarPos(0.0f, 0.0f, 0.0f);
glm::vec3 enemyHangarPos(2000.0f, 0.0f, 0.0f);

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
    enemyHangarPos + glm::vec3(0.0f, 20.0f, 0.0f),
    glm::vec3(500.0f, 200.0f, -100.0f),
    glm::vec3(600.0f, 200.0f, 100.0f),
    glm::vec3(1100.0f, 200.0f, 100.0f),
    glm::vec3(1600.0f, 200.0f, 100.0f),
    glm::vec3(1000.0f, 200.0f, -100.0f)
};
glm::vec3 starLightColor(0.7f, 0.8f, 1.0f);
glm::vec3 pointStarLightColor(0.1f, 0.2f, 1.0f);
glm::vec3 redLightColor(0.8f, 0.0f, 0.0f);
glm::vec3 blueLightColor(0.0f, 0.0f, 0.8f);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Fighter fighter_player(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 30.0f, 3, 10.0f);
// Inicialmente estão 3 estacionados
std::vector<Fighter> enemies = {
    Fighter(glm::vec3(750.0f, 30.0f, -80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
    Fighter(glm::vec3(600.0f, 20.0f, 0.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
    Fighter(glm::vec3(800.0f, 30.0f, 80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),

    Fighter(glm::vec3(2000.0f, 100.0f, -300.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f),
    Fighter(glm::vec3(2100.0f, 100.0f, 300.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f)
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
unsigned int VAOProjectile, numProjectileVertices, VBOProjectile;

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

ALuint buffer, source, buffer2, source2, buffer3, source3;
ALCdevice* device;
ALCcontext* context;

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
    // Create a 1x1 black pixel cursor
    unsigned char pixels[4] = { 0, 0, 0, 255 };
    GLFWimage image;
    image.width = 1;
    image.height = 1;
    image.pixels = pixels;
    GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
    glfwSetCursor(window, cursor);
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

    if(loadModel("models/xwing/xwing.obj", xwingModel, true) == -1)
        return -1;
    if (loadModel("models/hangar/hangar.obj", hangarModel, false) == -1)
        return -1;
    if(loadModel("models/tie/tie.obj", tieModel, true) == -1)
        return -1;
    loadLuz();
    loadText();

    // Build and compile shaders
    lightingShader = new Shader("shaders/light.vs", "shaders/light.fs");
    lightingCubeShader = new Shader("shaders/lamp.vs", "shaders/lamp.fs");
    textShader = new Shader("shaders/text.vs", "shaders/text.fs");
    skyboxShader = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    hitBoxShader = new Shader("shaders/hitbox.vs", "shaders/hitbox.fs");
    lightingPTexShader = new Shader("shaders/lightTex.vs", "shaders/lightTex.fs");
    
    // Setup skybox and load texture
    setupSkybox();
    setupBoundingBox();
    cubemapTexture = loadCubemap(faces);
    
    // Set skybox shader uniforms
    skyboxShader->use();
    skyboxShader->setInt("skybox", 0);

    glm::vec3 cameraOffset(0.0f, 10.0f, 60.0f);
    camera.Position = fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, cameraOffset.y, 0.0f);

    if(inicializarSound(device, context, buffer, buffer2, buffer3, source, source2, source3) == -1)
        return -1;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Loop the audio
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            alSourcePlay(source);
        }

        processInput();

        if(gameState == 5){
            renderIntro();
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

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

            // Spwan de inimigos -- deve parar se o player estiver perto do final
            static float lastSpawnTime = glfwGetTime();
            float currentSpawnTime = glfwGetTime();
            if (currentSpawnTime - lastSpawnTime >= 10.0f && fighter_player.position.x < enemyHangarPos.x - 500) {
                enemies.push_back(Fighter(enemyHangarPos + glm::vec3(0.0f, 5.0f, -80.0f), -fighter_player.front, 0.0f, 0.0f, 0.0f, 5.0f, 1, 10.0f));
                enemies.push_back(Fighter(enemyHangarPos + glm::vec3(-100.0f, 5.0f, 0.0f), -fighter_player.front, 0.0f, 0.0f, 0.0f, 5.0f, 1, 10.0f));
                enemies.push_back(Fighter(enemyHangarPos + glm::vec3(0.0f, 5.0f, 80.0f), -fighter_player.front, 0.0f, 0.0f, 0.0f, 5.0f, 1, 10.0f));
                animacaoInimigos();
                lastSpawnTime = currentSpawnTime;
            }

            static float lastTime = glfwGetTime();
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;

            createShot(0.2f, 0.5f, 36, VAOProjectile, numProjectileVertices);

            updateProjectiles(deltaTime);
            shootEnemyProjectiles(deltaTime);
            checkCollisions();

            renderProjectiles(*hitBoxShader);
        }
        // Em pausa (controlos)
        else if(gameState == 2){
            const std::string controlsTitle = "Game Controls";
            const std::string controlsText1 = "W - Increase Speed";
            const std::string controlsText2 = "S - Decrease Speed";
            const std::string controlsText3 = "Q - Attack";
            const std::string controlsText4 = "V - Toggle Camera View";
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
            float textWidth4 = controlsText4.length() * 25.0f;
            RenderText(controlsText4, (SCR_WIDTH - textWidth4) / 2.0f, SCR_HEIGHT / 2.0f - 80.0f, 1.0f, textColor, true);
            float textWidth6 = controlsText6.length() * 25.0f;
            RenderText(controlsText6, (SCR_WIDTH - textWidth6) / 2.0f, SCR_HEIGHT / 2.0f - 250.0f, 1.0f,  textColor, true);
            float textWidth7 = controlsText7.length() * 25.0f;
            RenderText(controlsText7, (SCR_WIDTH - textWidth7) / 2.0f, SCR_HEIGHT / 2.0f - 300.0f, 1.0f, textColor, true);
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

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // De-allocate resources
    delete skyboxShader;
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    glDeleteVertexArrays(1, &hangarModel.VAO);
    glDeleteBuffers(1, &hangarModel.VBO);
    glDeleteBuffers(1, &hangarModel.NBO);
    glDeleteBuffers(1, &hangarModel.TBO);
    glDeleteVertexArrays(1, &tieModel.VAO);
    glDeleteBuffers(1, &tieModel.VBO);
    glDeleteBuffers(1, &tieModel.NBO);
    glDeleteBuffers(1, &tieModel.TBO);
    glDeleteVertexArrays(1, &xwingModel.VAO);
    glDeleteBuffers(1, &xwingModel.VBO);
    glDeleteBuffers(1, &xwingModel.NBO);
    glDeleteBuffers(1, &xwingModel.TBO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteVertexArrays(1, &VAOt);
    glDeleteBuffers(1, &VBOt);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glfwTerminate();
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
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
int loadModel(const std::string& filePath, Model& model, bool centerModel) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> tinyMaterials;
    std::string warn, err;
    std::string mtlBaseDir = filePath.substr(0, filePath.find_last_of('/')) + "/";
    model.name = (filePath.substr(filePath.find_last_of('/') + 1)).substr(0, (filePath.substr(filePath.find_last_of('/') + 1)).find_last_of('.'));

    // Load the .obj file using TinyObjLoader
    if (!tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &err, filePath.c_str(), mtlBaseDir.c_str())) {
        std::cout << "Failed to load OBJ file: " << err << std::endl;
        return -1;
    }

    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }

    glm::vec3 center(0.0f);
    int totalVertices = 0;

    // Extract vertices, normals, and texture coordinates
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 vertex(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            );
            model.vertices.push_back(vertex);
            center += vertex;
            totalVertices++;
            if (!attrib.normals.empty()) {
                glm::vec3 normal(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                );
                model.normals.push_back(normal);
            }
            if (!attrib.texcoords.empty()) {
                glm::vec2 texCoord(
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                );
                model.texCoords.push_back(texCoord);
            }
        }
    }

    if (centerModel) {
        center /= totalVertices;
        for (auto& vertex : model.vertices) {
            vertex -= center;
        }
    }

    glGenVertexArrays(1, &model.VAO);
    glGenBuffers(1, &model.VBO);
    glGenBuffers(1, &model.NBO);
    glGenBuffers(1, &model.TBO);
    glBindVertexArray(model.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(glm::vec3), &model.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, model.NBO);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, model.TBO);
    glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(glm::vec2), &model.texCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(2);

    // Load materials if available
    if (!tinyMaterials.empty()) {
        std::string mtlFilePath = mtlBaseDir + model.name + ".mtl";
        std::cout << "Loading materials from: " << mtlFilePath << std::endl;
        loadMaterials(mtlFilePath, model.materials);
    } else {
        std::cout << "No materials found in the OBJ file." << std::endl;
    }

    return 0;
}

// Function to load materials and textures from MTL file
void loadMaterials(const std::string& mtlFilePath, std::vector<MaterialInfo>& materials) {
    std::ifstream mtlFile(mtlFilePath);
    if (!mtlFile) {
        std::cerr << "Failed to open MTL file: " << mtlFilePath << std::endl;
        return;
    }

    // Get the directory path of the MTL file
    std::string directory = mtlFilePath.substr(0, mtlFilePath.find_last_of('/'));

    std::map<std::string, int> material_map;
    std::vector<tinyobj::material_t> tinyMaterials;
    std::string warning;
    tinyobj::LoadMtl(&material_map, &tinyMaterials, &mtlFile, &warning);

    if (!warning.empty()) {
        std::cerr << "Warning: " << warning << std::endl;
    }

    for (const auto& tinyMaterial : tinyMaterials) {
        MaterialInfo material;
        material.name = tinyMaterial.name;
        material.diffuse_texname = tinyMaterial.diffuse_texname;
        material.specular_texname = tinyMaterial.specular_texname;
        material.normal_texname = tinyMaterial.bump_texname; // Use bump_texname for normal map

        // Load textures if they exist
        if (!material.diffuse_texname.empty()) {
            std::string texPath = directory + "/" + material.diffuse_texname;
            material.diffuse_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded diffuse texture: " << texPath << std::endl;
        }
        if (!material.specular_texname.empty()) {
            std::string texPath = directory + "/" + material.specular_texname;
            material.specular_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded specular texture: " << texPath << std::endl;
        }
        if (!material.normal_texname.empty()) {
            std::string texPath = directory + "/" + material.normal_texname;
            material.normal_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded normal texture: " << texPath << std::endl;
        }

        // Load additional textures
        if (!tinyMaterial.roughness_texname.empty()) {
            std::string texPath = directory + "/" + tinyMaterial.roughness_texname;
            material.specular_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded roughness texture: " << texPath << std::endl;
        }
        if (!tinyMaterial.metallic_texname.empty()) {
            std::string texPath = directory + "/" + tinyMaterial.metallic_texname;
            material.specular_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded metallic texture: " << texPath << std::endl;
        }
        if (!tinyMaterial.bump_texname.empty()) {
            std::string texPath = directory + "/" + tinyMaterial.bump_texname;
            material.normal_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded bump texture: " << texPath << std::endl;
        }

        materials.push_back(material);
        
        std::cout << "Material: " << material.name << std::endl;
        std::cout << "  Diffuse Texture: " << material.diffuse_texname << std::endl;
        std::cout << "  Specular Texture: " << material.specular_texname << std::endl;
        std::cout << "  Normal Texture: " << material.normal_texname << std::endl;
    }
}

unsigned int loadTexture(const char* path) {
    // Add this before loading texture
    stbi_set_flip_vertically_on_load(true);  // OpenGL expects texture coordinates to start from bottom-left

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
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

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
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
    float targetY = 20.0f;
    float speed = 0.5f; // speed of the animation

    while (fighter_player.position.y > targetY) {
        fighter_player.position.y -= speed; // increment position by speed

        // Render scene
        renderScene();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    fighter_player.position.y = targetY;
}

void moverInimigos() {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        // mover em frente
        glm::vec3 finalPos = glm::vec3(0.0f, fighter_player.position.y, 0.0f);
        glm::vec3 direction = glm::normalize(finalPos - it->position);
        glm::vec3 newPosition = it->position + direction * it->movementSpeed * 0.5f;
        it->position = newPosition;

        // Se colidiu morreu 
        // Verificar colisão com a nave principal
        if (isColliding(it->position, it->collisionRadius, fighter_player.position, fighter_player.collisionRadius)) {
            fighter_player.hp -= 1; // Reduzir 1 HP da nave principal
            it = enemies.erase(it); // Destruir a nave inimiga
            continue;
        }

        // if (glm::distance(newPosition, fighter_player.position) < 10.0f) {
        //     it = enemies.erase(it);
        //     continue;
        // }

        // Verificar se o inimigo está fora do ângulo de visão do player
        glm::vec3 toEnemy = glm::normalize(it->position - fighter_player.position);
        float dotProduct = glm::dot(fighter_player.front, toEnemy);
        float angleToPlayer = glm::degrees(glm::acos(dotProduct));
        if (angleToPlayer > 60.0f) {
            it = enemies.erase(it);
            continue;
        }

        // rodar na direção correta
        direction = glm::normalize(fighter_player.position - it->position);
        float targetYaw = glm::degrees(atan2(direction.z, -direction.x));
        float targetPitch = glm::degrees(asin(-direction.y));
        it->directionX = glm::mix(it->directionX, targetYaw, 0.05f);
        it->directionY = glm::mix(it->directionY, targetPitch, 0.05f);
        it->front = glm::normalize(glm::vec3(
            cos(glm::radians(it->directionY)) * cos(glm::radians(it->directionX)),
            sin(glm::radians(it->directionY)),
            cos(glm::radians(it->directionY)) * sin(glm::radians(it->directionX))
        ));

        ++it;
    }

    // distância minima entre os inimigos
    // for (size_t i = 0; i < enemies.size(); ++i) {
    //     for (size_t j = i + 1; j < enemies.size(); ++j) {
    //         if (glm::distance(enemies[i].position, enemies[j].position) < 100.0f) {
    //             glm::vec3 direction = glm::normalize(enemies[j].position - enemies[i].position);
    //             enemies[j].position = enemies[i].position + direction * 100.0f;
    //         }
    //     }
    // }
}

void animacaoInimigos(){
    float targetY = 30.0f;
    float speed = 0.5f; 

    bool allEnemiesAtTarget = false;
    while (!allEnemiesAtTarget) {
        allEnemiesAtTarget = true;
        for (std::vector<Fighter>::size_type i = enemies.size() - 4; i < enemies.size(); i++) {
            if (enemies[i].position.y < targetY) {
                enemies[i].position.y += speed;
                if (enemies[i].position.y > targetY) {
                    enemies[i].position.y = targetY;
                }
                allEnemiesAtTarget = false;
            }
        }

        processInput();
        renderScene();

        // Render text
        std::string vida = "Health: " + std::to_string(fighter_player.hp);
        std::string pontuacaoStr = "Points: " + std::to_string(pontuacao);
        RenderText(vida, SCR_WIDTH - 300.0f, SCR_HEIGHT - 100.0f, 1.0f, textColor, true);
        RenderText(pontuacaoStr, SCR_WIDTH - 300.0f, SCR_HEIGHT - 180.0f, 1.0f, textColor, true);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
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

    if(gameState == 5 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gameState = 0;

    if(gameState == 0 || gameState == 3 || gameState == 5)
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
            else if(cameraMode == 1)
                cameraMode = 2;
            else 
                cameraMode = 0;
            vKeyPressed = true;
        }
    } else {
        vKeyPressed = false;
    }

    static bool fireKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (!fireKeyPressed) {
            alSourcePlay(source2);
            shootProjectile(fighter_player);
            fireKeyPressed = true;
        }
        } else {
            fireKeyPressed = false;
        }

    if(cameraMode == 2){
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            fighter_player.position.z -= 5.0;
        }

        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            fighter_player.position.z += 5.0;
        }

        return;
    }

    static float lastPressTime = 0.0f;

    // Move forward with smooth acceleration
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (fighter_player.movementSpeed < 100.0f) {
            fighter_player.movementSpeed += 50.0f;
            alSourcePlay(source3);
        }
        lastPressTime = glfwGetTime();
    }
    // Move backward with smooth deceleration
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (fighter_player.movementSpeed > 30.0f) {
            fighter_player.movementSpeed -= 1.0f;
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
    if(cameraMode == 2)
        return;

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float xoffset = xpos - (width/2);
    float yoffset = (height/2) - ypos;

    float sensitivity = 0.005f;
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

        // Gradually reset the fighter's rotation to 0
        if (fighter_player.rotation > 0.0f) {
            fighter_player.rotation -= 0.5f; 
            if (fighter_player.rotation < 0.0f) {
                fighter_player.rotation = 0.0f;
            }
        } else if (fighter_player.rotation < 0.0f) {
            fighter_player.rotation += 0.5f; 
            if (fighter_player.rotation > 0.0f) {
                fighter_player.rotation = 0.0f;
            }
        }

        desenhaAlvo();
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
    else if (cameraMode == 1) {
        camera.Position = fighter_player.position;
    }
    else {
        glm::vec3 cameraOffset(100.0f, 300.0f, 0.0f); // Top view offset
        camera.Position = fighter_player.position + cameraOffset;
        camera.Front = glm::vec3(0.0f, -1.0f, 0.0f); // Look directly down
        camera.Up = glm::vec3(1.0f, 0.0f, 0.0f); // Adjust the up vector
        fighter_player.directionX = 0.0f;
        fighter_player.directionY = 0.0f;
        fighter_player.rotation = 0.0f;
    }

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    lightingShader->use();


    // Activate lightingPTexShader and set its uniforms
    lightsActivate(lightingPTexShader, view, projection);

    glm::mat4 model = glm::mat4(1.0f);

    // Render inimigos with lightingPTexShader
    for(Fighter enemy : enemies){
        model = glm::translate(glm::mat4(1.0f), enemy.position);
        model = glm::rotate(model, glm::radians(enemy.directionX + 90), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(3.0f));
        lightingPTexShader->setFloat("material.shininess", 32.0f);
        lightingPTexShader->setMat4("model", model);
        renderTextures(&xwingModel);
    }

    // Activate lightingShader and set its uniforms
    lightsActivate(lightingShader, view, projection);

    // Render hangars with lightingShader
    auto renderHangar = [&](glm::vec3 translation, float rotation) {
        model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader->setVec3("material.ambient", 0.3f, 0.3f, 0.3f);
        lightingShader->setVec3("material.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader->setVec3("material.specular", 0.6f, 0.6f, 0.6f);
        lightingShader->setFloat("material.shininess", 25.0f);
        lightingShader->setMat4("model", model);
        glBindVertexArray(hangarModel.VAO);
        glDrawArrays(GL_TRIANGLES, 0, hangarModel.vertices.size());
    };
    renderHangar(hangarPos, 0.0f);
    renderHangar(enemyHangarPos, 180.0f);

    // Render fighter with lightingShader
    model = glm::translate(glm::mat4(1.0f), fighter_player.position);
    model = glm::rotate(model, glm::radians(fighter_player.directionX + 270), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    model = glm::scale(model, glm::vec3(35.0f));
    lightingShader->setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
    lightingShader->setVec3("material.diffuse", 0.3f, 0.3f, 0.3f);
    lightingShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    lightingShader->setFloat("material.shininess", 32.0f);
    lightingShader->setMat4("model", model);
    glBindVertexArray(tieModel.VAO);
    glDrawArrays(GL_TRIANGLES, 0, tieModel.vertices.size());

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
        model = glm::translate(glm::mat4(1.0f), (pos + enemyHangarPos + glm::vec3(20.0f, 0.0f, 0.0f)));
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

void renderTextures(Model *Tmodel){
    glBindVertexArray(Tmodel->VAO);
    if (!Tmodel->materials.empty()) {
        // Bind vertex array before material loop
        bool anyMaterialApplied = false;
        for (const auto& material : Tmodel->materials){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, 0);
            if (!material.diffuse_texname.empty() || !material.specular_texname.empty() || !material.normal_texname.empty()) {
                anyMaterialApplied = true;
                glActiveTexture(GL_TEXTURE0);
                if (!material.diffuse_texname.empty()) {
                    glBindTexture(GL_TEXTURE_2D, material.diffuse_texid);
                    lightingPTexShader->setBool("hasDiffuseTexture", true);
                } else {
                    lightingPTexShader->setBool("hasDiffuseTexture", false);
                }

                glActiveTexture(GL_TEXTURE1);
                if (!material.specular_texname.empty()) {
                    glBindTexture(GL_TEXTURE_2D, material.specular_texid);
                    lightingPTexShader->setBool("hasSpecularTexture", true);
                } else {
                    lightingPTexShader->setBool("hasSpecularTexture", false);
                }

                glActiveTexture(GL_TEXTURE2);
                if (!material.normal_texname.empty()) {
                    glBindTexture(GL_TEXTURE_2D, material.normal_texid);
                    lightingPTexShader->setBool("hasNormalTexture", true);
                } else {
                    lightingPTexShader->setBool("hasNormalTexture", false);
                }
            }
        }
        // If no materials were applied, use default rendering
        if (!anyMaterialApplied) {
            lightingPTexShader->setBool("hasDiffuseTexture", false);
            lightingPTexShader->setBool("hasSpecularTexture", false);
            lightingPTexShader->setBool("hasNormalTexture", false);
            lightingPTexShader->setVec3("objectColor", 0.3f, 0.3f, 0.3f);
            glDrawArrays(GL_TRIANGLES, 0, Tmodel->vertices.size());
        }
    } else {
        // No materials, use default values
        lightingPTexShader->setBool("hasDiffuseTexture", false);
        lightingPTexShader->setBool("hasSpecularTexture", false);
        lightingPTexShader->setBool("hasNormalTexture", false);
        lightingPTexShader->setVec3("objectColor", 0.3f, 0.3f, 0.3f);
        std::cout << "No materials found for model" << std::endl;
    }

    glDrawArrays(GL_TRIANGLES, 0, Tmodel->vertices.size());

    // Unbind textures after rendering
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void lightsActivate(Shader *ls, glm::mat4 view, glm::mat4 projection){
    ls->use();
    ls->setMat4("projection", projection);
    ls->setMat4("view", view);
    ls->setVec3("viewPos", camera.Position);

    // directional light
    ls->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    ls->setVec3("dirLight.ambient", starLightColor * glm::vec3(0.1f));
    ls->setVec3("dirLight.diffuse", starLightColor * glm::vec3(0.5f));
    ls->setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);

    // point light 1
    ls->setVec3("pointLights[0].position", pointLightPositions[0]);
    ls->setVec3("pointLights[0].ambient", blueLightColor * glm::vec3(0.3f));
    ls->setVec3("pointLights[0].diffuse", blueLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[0].specular", 0.6f, 0.6f, 0.6f);
    ls->setFloat("pointLights[0].constant", 1.0f);
    ls->setFloat("pointLights[0].linear", 0.002f);
    ls->setFloat("pointLights[0].quadratic", 0.0001f);
    // point light 2
    ls->setVec3("pointLights[1].position", pointLightPositions[1]);
    ls->setVec3("pointLights[1].ambient", redLightColor * glm::vec3(0.3f));
    ls->setVec3("pointLights[1].diffuse", redLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[1].specular", 0.6f, 0.6f, 0.6f);
    ls->setFloat("pointLights[1].constant", 1.0f);
    ls->setFloat("pointLights[1].linear", 0.002f);
    ls->setFloat("pointLights[1].quadratic", 0.0001f);
    // point light 3
    ls->setVec3("pointLights[2].position", pointLightPositions[2]);
    ls->setVec3("pointLights[2].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[2].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[2].specular", 0.8f, 0.8f, 0.8f);
    ls->setFloat("pointLights[2].constant", 1.0f);
    ls->setFloat("pointLights[2].linear", 0.014f);
    ls->setFloat("pointLights[2].quadratic", 0.0007f);
    // point light 4
    ls->setVec3("pointLights[3].position", pointLightPositions[3]);
    ls->setVec3("pointLights[3].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[3].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[3].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[3].constant", 1.0f);
    ls->setFloat("pointLights[3].linear", 0.014f);
    ls->setFloat("pointLights[3].quadratic", 0.0007f);
    // point light 5
    ls->setVec3("pointLights[4].position", pointLightPositions[4]);
    ls->setVec3("pointLights[4].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[4].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[4].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[4].constant", 1.0f);
    ls->setFloat("pointLights[4].linear", 0.014f);
    ls->setFloat("pointLights[4].quadratic", 0.0007f);
    // point light 6
    ls->setVec3("pointLights[5].position", pointLightPositions[5]);
    ls->setVec3("pointLights[5].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[5].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[5].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[5].constant", 1.0f);
    ls->setFloat("pointLights[5].linear", 0.014f);
    ls->setFloat("pointLights[5].quadratic", 0.0007f);
    // point light 7
    ls->setVec3("pointLights[6].position", pointLightPositions[6]);
    ls->setVec3("pointLights[6].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[6].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[6].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[6].constant", 1.0f);
    ls->setFloat("pointLights[6].linear", 0.014f);
    ls->setFloat("pointLights[6].quadratic", 0.0007f);

    // spot lights
    int i = 0;
    for (glm::vec3 pos : spotLightPositions) {
        ls->setVec3("spotLights[" + std::to_string(i) + "].position", pos);
        ls->setVec3("spotLights[" + std::to_string(i) + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ls->setFloat("spotLights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(25.0f)));
        ls->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(40.0f)));
        ls->setVec3("spotLights[" + std::to_string(i) + "].ambient", 0.4f, 0.4f, 0.4f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].linear", 0.014f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.0007f);
        i++;
    }
    // spot lights do segundo hangar
    i = 9;
    for (glm::vec3 pos : spotLightPositions) {
        ls->setVec3("spotLights[" + std::to_string(i) + "].position", (pos + enemyHangarPos + glm::vec3(20.0f, 0.0f, 0.0f)));
        ls->setVec3("spotLights[" + std::to_string(i) + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ls->setFloat("spotLights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(25.0f)));
        ls->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(40.0f)));
        ls->setVec3("spotLights[" + std::to_string(i) + "].ambient", 0.4f, 0.4f, 0.4f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].linear", 0.014f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.0007f);
        i++;
    }
}

void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, bool useFirstFont = true)
{
    textShader->use();
    glm::mat4 projection;
    if(gameState == 5){
        projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    } else {
        projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    }
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

bool isColliding(glm::vec3 projPosition, float projRadius, glm::vec3 enemyPosition, float enemyRadius) {
    float distance = glm::distance(projPosition, enemyPosition);
    std::cout << "Distance between projectile and enemy: " << distance << std::endl;
    std::cout << "Sum of radii: " << (projRadius + enemyRadius) << std::endl;
    return distance < (projRadius + enemyRadius);
}

// void checkCollisions() {
//     for (auto it = projectiles.begin(); it != projectiles.end();) {
//         bool collisionDetected = false;

//         // Verifica colisão com inimigos
//         for (auto& enemy : enemies) {
//             if (enemy.hp > 0 && isColliding(it->position, it->collisionRadius, enemy.position, enemy.collisionRadius)) {
//                 enemy.hp -= 1; // Reduz vida do inimigo
//                 if (enemy.hp <= 0) {
//                     pontuacao += 10; // Incrementa pontuação se o inimigo foi destruído
//                 }
//                 collisionDetected = true;
//                 break;
//             }
//         }

//         // Verifica colisão com o jogador
//         if (isColliding(it->position, it->collisionRadius, fighter_player.position, fighter_player.collisionRadius)) {
//             fighter_player.hp -= 1; // Reduz vida do jogador
//             collisionDetected = true;
//         }

//         if (collisionDetected) {
//             it = projectiles.erase(it); // Remove projétil após a colisão
//         } else {
//             ++it;
//         }
//     }
// }

void checkCollisions() {
    for (auto projIt = projectiles.begin(); projIt != projectiles.end();) {
        bool collisionDetected = false;

        // Verifica colisões com inimigos
        if (projIt->origin == 0) { // Projétil do jogador
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                std::cout << "Checking collision between projectile at (" 
                          << projIt->position.x << ", " << projIt->position.y << ", " << projIt->position.z 
                          << ") with radius " << projIt->collisionRadius 
                          << " and enemy at (" 
                          << enemyIt->position.x << ", " << enemyIt->position.y << ", " << enemyIt->position.z 
                          << ") with radius " << enemyIt->collisionRadius << std::endl;

                float distance = glm::distance(projIt->position, enemyIt->position);
                std::cout << "Distance between projectile and enemy: " << distance << std::endl;
                std::cout << "Sum of radii: " << (projIt->collisionRadius + enemyIt->collisionRadius) << std::endl;

                if (isColliding(projIt->position, projIt->collisionRadius, enemyIt->position, enemyIt->collisionRadius)) {
                    std::cout << "Collision detected between projectile and enemy" << std::endl;
                    enemyIt->hp -= 1; // Reduz a vida do inimigo

                    if (enemyIt->hp <= 0) {
                        std::cout << "Enemy destroyed" << std::endl;
                        pontuacao += 10; // Incrementa pontuação
                        enemyIt = enemies.erase(enemyIt); // Remove o inimigo destruído
                    } else {
                        ++enemyIt;
                    }

                    collisionDetected = true;
                    break; // Projétil já colidiu, sair do loop
                } else {
                    ++enemyIt;
                }
            }
        } else { // Projétil do inimigo
            std::cout << "Checking collision between projectile at (" 
                      << projIt->position.x << ", " << projIt->position.y << ", " << projIt->position.z 
                      << ") with radius " << projIt->collisionRadius 
                      << " and player at (" 
                      << fighter_player.position.x << ", " << fighter_player.position.y << ", " << fighter_player.position.z 
                      << ") with radius " << fighter_player.collisionRadius << std::endl;

            float distance = glm::distance(projIt->position, fighter_player.position);
            std::cout << "Distance between projectile and player: " << distance << std::endl;
            std::cout << "Sum of radii: " << (projIt->collisionRadius + fighter_player.collisionRadius) << std::endl;

            if (isColliding(projIt->position, projIt->collisionRadius, fighter_player.position, fighter_player.collisionRadius)) {
                std::cout << "Collision detected between projectile and player" << std::endl;
                fighter_player.hp -= 1; // Reduz a vida do jogador
                collisionDetected = true;
            }
        }

        if (collisionDetected) {
            std::cout << "Projectile removed" << std::endl;
            projIt = projectiles.erase(projIt); // Remove o projétil que colidiu
        } else {
            ++projIt;
        }
    }
}

void renderBoundingBox(glm::vec3 position, float radius, float scaleFactor) {
    // std::cout << "Rendering bounding box at position: " 
    //           << position.x << ", " << position.y << ", " << position.z 
    //           << " with radius: " << radius << std::endl;

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

void renderProjectiles(Shader& shader) {
    shader.use();

    for (const auto& proj : projectiles) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), proj.position);
        model = glm::scale(model, glm::vec3(2.0f, 7.0f, 2.0f)); // Escala aumentada para maior visibilidade
        shader.setMat4("model", model);

        if (proj.origin == 0) {
            shader.setVec3("boxColor", glm::vec3(0.0f, 1.0f, 0.0f)); // Verde para o jogador
        } else {
            shader.setVec3("boxColor", glm::vec3(1.0f, 0.0f, 0.0f)); // Vermelho para inimigos
        }

        // Configure as matrizes de visão e projeção
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f));

        glBindVertexArray(VAOProjectile);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numProjectileVertices); // 2 * (36 segmentos)
        glBindVertexArray(0);

        // std::cout << "Projectile rendered at position: " << proj.position.x << ", " << proj.position.y << ", " << proj.position.z << std::endl;

    }
    // std::cout << "Projectiles rendered. Count: " << projectiles.size() << std::endl;

}


void shootProjectile(const Fighter& fighter) {
    glm::vec3 projectilePosition = fighter.position + fighter.front * 5.0f;
    glm::vec3 direction = fighter.front;

    // Encontre o inimigo mais próximo
    Fighter* closestEnemy = findClosestEnemy(fighter.position);
    if (closestEnemy) {
        float distance = glm::distance(fighter.position, closestEnemy->position);
        float maxAutoAimDistance = 500.0f; // Defina a distância máxima para a mira automática

        if (distance < maxAutoAimDistance) {
            direction = glm::normalize(closestEnemy->position - fighter.position);
        }
    }

    // Adiciona um novo projétil ao vetor
    projectiles.emplace_back(projectilePosition, direction, 50.0f, 2.0f, 0); // Velocidade ajustada para 50.0f

    std::cout << "Projectile shot from position: " << projectilePosition.x << ", " << projectilePosition.y << ", " << projectilePosition.z << std::endl;
}


void updateProjectiles(float deltaTime) {
    for (auto& proj : projectiles) {
        proj.position += proj.direction * proj.speed * deltaTime * 0.5f;
        std::cout << "Projectile updated to position: " << proj.position.x << ", " << proj.position.y << ", " << proj.position.z << std::endl;
    }

    // Remover projéteis que saíram dos limites
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& proj) {
        return proj.position.z > 700.0f || proj.position.z < -700.0f; // Limites arbitrários
    }), projectiles.end());

    std::cout << "Projectiles updated. Count: " << projectiles.size() << std::endl;
}

void shootEnemyProjectiles(float deltaTime) {
    static float lastShootTime = 0.0f;
    float currentTime = glfwGetTime();

    // Atira a cada 2 segundos
    if (currentTime - lastShootTime >= 3.0f) {
        for (const auto& enemy : enemies) {
            glm::vec3 projectilePosition = enemy.position + enemy.front * 5.0f; // Posição inicial
            glm::vec3 projectileDirection = enemy.front;

            float distance = glm::distance(enemy.position, fighter_player.position);
            float maxAutoAimDistance = 500.0f; // Defina a distância máxima para a mira automática

            if (distance < maxAutoAimDistance) {
                projectileDirection = glm::normalize(fighter_player.position - enemy.position);
            }

            // Velocidade reduzida para projéteis inimigos
            float projectileSpeed = 20.0f; // Ajuste conforme necessário
            projectiles.emplace_back(projectilePosition, projectileDirection, projectileSpeed, 2.0f, 1);

            std::cout << "Enemy projectile shot from position: " << projectilePosition.x << ", " << projectilePosition.y << ", " << projectilePosition.z << std::endl;
        }
        lastShootTime = currentTime;
    }
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

void createShot(float radius, float height, int segments, unsigned int& VAO, unsigned int& VBO){
    std::vector<float> shotVertices;

    // Gerar vértices para as tampas superior e inferior
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Parte inferior
        shotVertices.push_back(x); // x
        shotVertices.push_back(-height / 2.0f); // y
        shotVertices.push_back(z); // z

        // Parte superior
        shotVertices.push_back(x); // x
        shotVertices.push_back(height / 2.0f); // y
        shotVertices.push_back(z); // z
    }

    // Configuração do VBO e VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, shotVertices.size() * sizeof(float), shotVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Fighter* findClosestEnemy(const glm::vec3& playerPosition) {
    Fighter* closestEnemy = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (auto& enemy : enemies) {
        if (enemy.hp > 0) { // Considere apenas inimigos vivos
            float distance = glm::distance(playerPosition, enemy.position);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestEnemy = &enemy;
            }
        }
    }
    return closestEnemy;
}

void desenhaAlvo(){
    // Desabilitar o teste de profundidade para desenhar a mira na frente de tudo
    glDisable(GL_DEPTH_TEST);

    // Salvar a matriz de projeção e visualização atuais
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Definir a cor da mira (branco)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Definir a largura da linha
    glLineWidth(2.0f);

    // Desenhar a mira no centro da tela
    glBegin(GL_LINES);
    // Linha horizontal
    glVertex2f(0.49f, 0.5f);
    glVertex2f(0.51f, 0.5f);
    // Linha vertical
    glVertex2f(0.5f, 0.49f);
    glVertex2f(0.5f, 0.51f);
    glEnd();

    // Restaurar a matriz de projeção e visualização
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Habilitar o teste de profundidade novamente
    glEnable(GL_DEPTH_TEST);
}

void restartGame(){
    cameraMode = 0;
    pontuacao = 0;
    enemies = {
        Fighter(glm::vec3(750.0f, 30.0f, -80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
        Fighter(glm::vec3(600.0f, 20.0f, 0.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
        Fighter(glm::vec3(800.0f, 30.0f, 80.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),

        Fighter(glm::vec3(2000.0f, 100.0f, -300.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f),
        Fighter(glm::vec3(2100.0f, 100.0f, 300.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f)
    };

    camera = glm::vec3(0.0f, 0.0f, 0.0f);
    fighter_player = Fighter(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 30.0f, 3, 10.0f);
    projectiles.clear();

    animacaoSaida();
}

void renderIntro(){
    // Clear the screen
    glClearColor(0.01f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const std::string introText[] = {
        "The   Rebel   Alliance,   battered   but   defiant   has,     ",
        "established   a   hidden   staging     ground    in   the     ",
        "remote   Outer   Rim.   But   the   relentless Imperial       ",
        "Fleet,   under  the  command  of  the  cunning  Grand         ",
        "Admiral   Thrawn,   has   discovered   their   location.      ",
        "Imperial    intelligence   has    uncovered   a   critical    ",
        "weakness   in  the   Rebel  defenses:  the  vulnerable        ",
        "hangar   bay   of   the   Cruiser   Liberator.   A    swift   ",
        "strike   against  this   crucial   vessel   could   cripple   ",
        "the   Rebel  fleet   before   it  can  launch  a  counter-    ",
        "offensive. Now,  as  the   Imperial  armada  descends         ",
        "from    the     stars,   the    fate    of   the     Rebellion",
        "hangs   in   the   balance . . .                              "
    };

    static float scrollOffset = 0.0f;
    scrollOffset += 5.0f; // Adjust the speed of scrolling here

    for (int i = 0; i < 13; ++i) {
        float textWidth = introText[i].length() * 20.0f;
        float yPos = SCR_HEIGHT / 2.0f - 200.0f - i * 80.0f + scrollOffset; 
        if (yPos > -50.0f && yPos < SCR_HEIGHT + 50.0f) {
            RenderText(introText[i], (SCR_WIDTH - textWidth) / 2.0f, yPos, 1.0f, textColor, true);
        }
    }

    if (scrollOffset > 160.0f + 13 * 80.0f) {
        scrollOffset = 0.0f; // Reset the scroll after the text has scrolled off the screen
        gameState = 0; // Change to game mode 0
    }
}

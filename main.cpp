#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "headers/common.h"
#include "headers/loaders.h"
#include "headers/structs.h"
#include "headers/setups.h"
#include "headers/input.h"
#include "headers/audio.h"
#include "headers/render.h"
#include "headers/acoes.h"
#include "headers/texto.h"

#include FT_FREETYPE_H

// Variáveis globais
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWwindow* window;

// Modelos
Model hangarModel;
Model tieModel;
Model xwingModel;

std::map<std::string, std::vector<MaterialInfo>> modelMaterials;
std::vector<Explosion> explosions;
unsigned int explosionTextureID;

// Shaders
Shader* skyboxShader;
Shader* lightingShader;
Shader* lightingCubeShader;
Shader* textShader;
Shader* hitBoxShader;
Shader* explosionShader;
Shader* lightingPTexShader;

// Variáveis de jogo
unsigned int cameraMode = 0;
unsigned int gameState = 5;
int pontuacao = 0;

// Coordenadas dos dois hangares
glm::vec3 hangarPos(0.0f, 0.0f, 0.0f);
glm::vec3 enemyHangarPos(2000.0f, 0.0f, 0.0f);

// Coordenadas das diferentes fontes de luz
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
// Diferentes cores de luz
glm::vec3 starLightColor(0.7f, 0.8f, 1.0f);
glm::vec3 pointStarLightColor(0.1f, 0.2f, 1.0f);
glm::vec3 redLightColor(0.8f, 0.0f, 0.0f);
glm::vec3 blueLightColor(0.0f, 0.0f, 0.8f);

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Naves
Fighter fighter_player(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 30.0f, 3, 10.0f);
std::vector<Fighter> enemies = {
    Fighter(glm::vec3(750.0f, 50.0f, -90.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
    Fighter(glm::vec3(600.0f, 20.0f, 0.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
    Fighter(glm::vec3(800.0f, 50.0f, 90.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),

    Fighter(glm::vec3(2000.0f, 100.0f, -200.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f),
    Fighter(glm::vec3(2100.0f, 100.0f, 200.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f)
};

// Projeteis
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
unsigned int exVAO, exVBO;

// Textura do skybox
unsigned int cubemapTexture; 
std::vector<std::string> faces = {
    "models/sky/skyboxxa.png",
    "models/sky/skyboxxb.png",
    "models/sky/skyboxya.png",
    "models/sky/skyboxyb.png",
    "models/sky/skyboxza.png",
    "models/sky/skyboxzb.png"
};

// Variáveis para o texto
std::map<GLchar, Character> Characters, Characters2;
unsigned int VAOt, VBOt;
glm::vec3 textColor = glm::vec3(255.0f / 255.0f, 232.0f / 255.0f, 31.0f / 255.0f);

// Variáveis para os efeitos sonoros
ALuint buffer, source, buffer2, source2, buffer3, source3, buffer4, source4;
ALCdevice* device;
ALCcontext* context;

/**
 * @brief A função principal inicializa e configura o GLFW, cria uma janela em fullscreen,
 * usa o GLAD para permitir o uso de todas as funções do OpenGL e é configurado o estado global do mesmo.
 * São compilados e construídos os shaders recorrendo á classe Shader. 
 * Depois carrega os três modelos (hangar, nave do jogador e nave inimiga).
 * Depois destes passos e de inicializar todas as variáveis globais necessárias, 
 * começa então o ciclo de renderização. 
 * 
 * A cada iteração do ciclo é verificado se a música de fundo continua a ser reproduzida 
 * e são verificados os eventos de input. Dependendo do estado atual do jogo serão rederizadas diferentes componentes
 * e realizadas diferentes operações.
 * No fim de cada iteração, troca os buffers e pesquisa eventos de IO até que a janela deva fechar.
 * 
 * Após o loop, desaloca recursos e termina o GLFW.
 * 
 * @return int - Retorna 0 se foi bem sucedido, ou -1 se ocorreu algum erro.
 */
int main() {
    // Inicialização e configuração do GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criação da janela GLFW no monitor principal
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    // Esta janela terá tamanho correspondente ao monitor de modo a abrir em tela cheia
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
    // Configuração do cursor para que este não seja perceptivel
    unsigned char pixels[4] = { 0, 0, 0, 255 };
    GLFWimage image;
    image.width = 1;
    image.height = 1;
    image.pixels = pixels;
    GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
    glfwSetCursor(window, cursor);
    glfwSetCursorPos(window, SCR_WIDTH/2, SCR_HEIGHT/2);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Carregar ponteiros de função OpenGL usando o GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Configurar o estado global do OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Carergar modelos
    if(loadModel("models/xwing/xwing.obj", xwingModel, true) == -1)
        return -1;
    if (loadModel("models/hangar/hangar.obj", hangarModel, false) == -1)
        return -1;
    if(loadModel("models/tie/tie.obj", tieModel, true) == -1)
        return -1;
    loadLuz();
    loadText();

    // Construir e compilar os shaders
    lightingShader = new Shader("shaders/light.vs", "shaders/light.fs");
    lightingCubeShader = new Shader("shaders/lamp.vs", "shaders/lamp.fs");
    textShader = new Shader("shaders/text.vs", "shaders/text.fs");
    skyboxShader = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    hitBoxShader = new Shader("shaders/hitbox.vs", "shaders/hitbox.fs");
    lightingPTexShader = new Shader("shaders/lightTex.vs", "shaders/lightTex.fs");
    explosionShader = new Shader("shaders/explosion.vs", "shaders/explosion.fs");

    // Carregar textura da explosão
    explosionTextureID = loadTexture("models/explosion.png");

    setupSquare();
    
    // Setup da skybox
    setupSkybox();
    setupBoundingBox();
    cubemapTexture = loadCubemap(faces);
   
    skyboxShader->use();
    skyboxShader->setInt("skybox", 0);

    // Configuração inicial da posição da camera, para a perspetiva de terceira pessoa
    glm::vec3 cameraOffset(0.0f, 10.0f, 60.0f);
    camera.Position = fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, cameraOffset.y, 0.0f);

    if(inicializarSound(device, context, buffer, buffer2, buffer3, buffer4, source, source2, source3, source4) == -1)
        return -1;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // De modo a que a música de fundo nunca pare é necessário reproduzir o audio assim que este termina
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            alSourcePlay(source);
        }

        // Processar entradas do teclado
        processInput();

        // Caso o jogo esteja no estado de introdução deverá ser renderizada apenas a tela de introdução
        if(gameState == 5){
            renderIntro();
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        // Em qualquer outro estado do jogo deverá ser renderizada a cena do jogo
        renderScene();
        
        // Se o jogo está no estado inicial
        if(gameState == 0){
            textoInicio();
            checkStart();
        }
        
        // Se o jogo está no estado de decorrer
        else if(gameState == 1){
            textoDurante();
            moverInimigos();

            // Verificar se o jogo terminou
            // o jogo termina se o jogador ficou sem vida, ou se já alcançou o hangar inimigo
            if(fighter_player.position.x >= enemyHangarPos.x - 100)
                gameState = 4;
            else if (fighter_player.hp == 0)
                gameState = 4;

            // A cada 10 segundos gerar novos inimigos na cena saídos do hangar inimigo
            static float lastSpawnTime = glfwGetTime();
            float currentSpawnTime = glfwGetTime();
            // só deve acontecer caso o jogador ainda esteja afastado do hangar inimigo
            if (currentSpawnTime - lastSpawnTime >= 10.0f && fighter_player.position.x < enemyHangarPos.x - 500) {
                enemies.push_back(Fighter(enemyHangarPos + glm::vec3(0.0f, 5.0f, -100.0f), -fighter_player.front, 0.0f, 0.0f, 0.0f, 5.0f, 1, 10.0f));
                enemies.push_back(Fighter(enemyHangarPos + glm::vec3(-100.0f, 5.0f, 0.0f), -fighter_player.front, 0.0f, 0.0f, 0.0f, 5.0f, 1, 10.0f));
                enemies.push_back(Fighter(enemyHangarPos + glm::vec3(0.0f, 5.0f, 100.0f), -fighter_player.front, 0.0f, 0.0f, 0.0f, 5.0f, 1, 10.0f));
                animacaoInimigos();
                lastSpawnTime = currentSpawnTime;
            }

            // Para o funcionamento dos tiros
            static float lastTime = glfwGetTime();
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;

            createShot(0.2f, 0.5f, 36, VAOProjectile, numProjectileVertices);

            updateProjectiles(deltaTime);
            shootEnemyProjectiles(deltaTime);
            checkCollisions();

            renderProjectiles(*hitBoxShader);
        }
        // Se o jogo está no estado de pausa
        else if(gameState == 2){
            textoPausa();
        }
        // Se o jogo está no estado final
        else if(gameState == 4){
            textoFinal();
        }
        
        // Trocar buffers e verificar eventos de I/O
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Libertar recursos
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
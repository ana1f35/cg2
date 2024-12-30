#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "include/shader_m.h"
#include "include/camera.h"
#include "include/tiny_obj_loader.h"
#include "include/stb_image.h"
#include <iostream>
#include <vector>


// Declaração das funções
int loadModel(const std::string& filePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, unsigned int& VAO, unsigned int& VBO, unsigned int& NBO, unsigned int& TBO, bool centerModel);
void renderScene(Shader& lightingShader);
bool checkStart(Shader& lightingShader);
void animacaoSaida(Shader& lightingShader);
void animacaoAterragem(Shader& lightingShader, glm::vec3 ponto);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput();

// Variáveis globais
unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;
GLFWwindow* window;

// Lighting
glm::vec3 lightPos(20.0f, 50.0f, 0.0f);
glm::vec3 lightPos2(430.0f, 50.0f, 0.0f);
glm::vec3 redLightColor(1.0f, 1.0f, 1.0f);
glm::vec3 blueLightColor(1.0f, 1.0f, 1.0f);

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
        
    Fighter(glm::vec3 pos, glm::vec3 frnt, float rotation, float directionX, float directionY) 
        : position(pos), front(frnt), rotation(rotation), directionX(directionX), directionY(directionY) {}
};
Fighter fighter_player(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch);
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
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> texCoords;
unsigned int VBO, VAO, NBO, TBO;
std::vector<glm::vec3> vertices2;
std::vector<glm::vec3> normals2;
std::vector<glm::vec2> texCoords2;
unsigned int VBO2, VAO2, NBO2, TBO2;

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
    // SCR_WIDTH = 1900;
    // SCR_HEIGHT = 800;
    // GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "StarWars", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    
    // Build and compile shaders
    Shader lightingShader("shaders/tex.vs", "shaders/tex.fs");

    // if (loadModel("texmodels/hangar/hangar.obj", vertices, normals, texCoords, VAO, VBO, NBO, TBO, false) == -1)
    //     return -1;
    if (loadModel("models/xwing/xwing.obj", vertices2, normals2, texCoords2, VAO2, VBO2, NBO2, TBO2, true) == -1)
        return -1;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput();
        // // Verificar se está perto de um ponto de aterragem
        // if(checkLanding(lightingShader));
        // // Se não aterrou, então verificar a partida
        // else
        //     checkStart(lightingShader);
        // Render scene
        renderScene(lightingShader);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // De-allocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &NBO2);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glfwTerminate();
    return 0;
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
// int loadModel(const std::string& filePath, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, unsigned int& VAO, unsigned int& VBO, unsigned int& NBO, unsigned int& TBO, bool centerModel) {
//     tinyobj::attrib_t attrib;
//     std::vector<tinyobj::shape_t> shapes;
//     std::vector<tinyobj::material_t> materials;
//     std::string warn, err;
//     std::string baseDir = filePath.substr(0, filePath.find_last_of('/')) + "/";

//     // Load the .obj file using TinyObjLoader
//     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath.c_str(), baseDir.c_str())) {
//         std::cout << "Failed to load OBJ file: " << err << std::endl;
//         return -1;
//     }

//     if (!err.empty()) {
//         std::cout << "Error: " << err << std::endl;
//     }

//     std::cout << "Loaded " << shapes.size() << " shapes" << std::endl;
//     std::cout << "Loaded " << materials.size() << " materials" << std::endl;

//     glm::vec3 center(0.0f);
//     int totalVertices = 0;

//     // Extract vertices, normals, and texture coordinates for each point of the model
//     for (const auto& shape : shapes) {
//         for (const auto& index : shape.mesh.indices) {
//             glm::vec3 vertex(
//                 attrib.vertices[3 * index.vertex_index + 0],
//                 attrib.vertices[3 * index.vertex_index + 1],
//                 attrib.vertices[3 * index.vertex_index + 2]
//             );
//             vertices.push_back(vertex);
//             center += vertex;
//             totalVertices++;
//             if (!attrib.normals.empty()) {
//                 glm::vec3 normal(
//                     attrib.normals[3 * index.normal_index + 0],
//                     attrib.normals[3 * index.normal_index + 1],
//                     attrib.normals[3 * index.normal_index + 2]
//                 );
//                 normals.push_back(normal);
//             }
//             if (!attrib.texcoords.empty()) {
//                 glm::vec2 texCoord(
//                     attrib.texcoords[2 * index.texcoord_index + 0],
//                     attrib.texcoords[2 * index.texcoord_index + 1]
//                 );
//                 texCoords.push_back(texCoord);
//             }
//         }
//     }

//     if (centerModel) {
//         center /= totalVertices;
//         for (auto& vertex : vertices) {
//             vertex -= center;
//         }
//     }

//     glGenVertexArrays(1, &VAO);
//     glGenBuffers(1, &VBO);
//     glGenBuffers(1, &NBO);
//     glGenBuffers(1, &TBO);
//     glBindVertexArray(VAO);

//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
//     glEnableVertexAttribArray(0);

//     glBindBuffer(GL_ARRAY_BUFFER, NBO);
//     glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
//     glEnableVertexAttribArray(1);

//     glBindBuffer(GL_ARRAY_BUFFER, TBO);
//     glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), &texCoords[0], GL_STATIC_DRAW);
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
//     glEnableVertexAttribArray(2);

//     // Load textures
//     auto loadTexture = [&](const std::string& texturePath) {
//         unsigned int textureID;
//         glGenTextures(1, &textureID);
//         glBindTexture(GL_TEXTURE_2D, textureID);

//         int width, height, nrComponents;
//         unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrComponents, 0);
//         if (data) {
//             GLenum format;
//             if (nrComponents == 1)
//                 format = GL_RED;
//             else if (nrComponents == 3)
//                 format = GL_RGB;
//             else if (nrComponents == 4)
//                 format = GL_RGBA;

//             glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//             glGenerateMipmap(GL_TEXTURE_2D);

//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//             stbi_image_free(data);
//         } else {
//             std::cout << "Failed to load texture: " << texturePath << std::endl;
//             stbi_image_free(data);
//         }
//         return textureID;
//     };

//     for (const auto& material : materials) {
//         std::cout << "Material name: " << material.name << std::endl;
//         if (!material.diffuse_texname.empty()) {
//             std::string texturePath = baseDir + material.diffuse_texname;
//             std::cout << "Loading texture: " << texturePath << std::endl;
//             loadTexture(texturePath);
//         }
//         if (!material.metallic_texname.empty()) {
//             std::string texturePath = baseDir + material.metallic_texname;
//             std::cout << "Loading texture: " << texturePath << std::endl;
//             loadTexture(texturePath);
//         }
//         if (!material.roughness_texname.empty()) {
//             std::string texturePath = baseDir + material.roughness_texname;
//             std::cout << "Loading texture: " << texturePath << std::endl;
//             loadTexture(texturePath);
//         }
//         if (!material.normal_texname.empty()) {
//             std::string texturePath = baseDir + material.normal_texname;
//             std::cout << "Loading texture: " << texturePath << std::endl;
//             loadTexture(texturePath);
//         }
//     }

//     return 0;
// }
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

    // Extract vertices, normals, and texture coordinates for each point of the model
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

    // Center the model if required
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

    // Create VBO for vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Create VBO for normals
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    // Create VBO for texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), &texCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(2);

    // Load textures for each material
    std::map<std::string, int> material_map;
    std::ifstream mtlStream(baseDir + "your_mtl_file.mtl");
    if (mtlStream) {
        tinyobj::LoadMtl(&material_map, &materials, &mtlStream, &warn);
    }

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

    // Iterate through each material and load textures
    for (const auto& material : materials) {
        std::cout << "Material name: " << material.name << std::endl;

        // Check and load diffuse texture if available
        if (!material.diffuse_texname.empty()) {
            std::string texturePath = baseDir + material.diffuse_texname;
            std::cout << "Loading diffuse texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }

        // Check and load metallic texture if available
        if (!material.metallic_texname.empty()) {
            std::string texturePath = baseDir + material.metallic_texname;
            std::cout << "Loading metallic texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }

        // Check and load roughness texture if available
        if (!material.roughness_texname.empty()) {
            std::string texturePath = baseDir + material.roughness_texname;
            std::cout << "Loading roughness texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }

        // Check and load normal texture if available
        if (!material.normal_texname.empty()) {
            std::string texturePath = baseDir + material.normal_texname;
            std::cout << "Loading normal texture: " << texturePath << std::endl;
            loadTexture(texturePath);
        }
    }

    return 0;
}

/**
 * @brief Esta função verifica se o fighter está num estado de "estacionado" e caso esteja e a tecla de espaço for pressionada, é acionada a animação de partida.
 *
 * @param lightingShader Referência ao shader usado para efeitos de iluminação.
 * @return bool - true se tiver sido realizada a partida, ou false caso contrário.
 */
bool checkStart(Shader& lightingShader) {
    if(estacionado != 0){
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            animacaoSaida(lightingShader);
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
void animacaoSaida(Shader& lightingShader){
    estacionado = 0;
    float startY = fighter_player.position.y;
    float endY = 20.0f;
    float duration = 2.0f; // duration in seconds
    float startTime = glfwGetTime();
    
    while (glfwGetTime() - startTime < duration) {
        float currentTime = glfwGetTime();
        float t = (currentTime - startTime) / duration;
        fighter_player.position.y = glm::mix(startY, endY, t);
        
        // Render scene
        renderScene(lightingShader);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    fighter_player.position.y = endY;
}

/**
 * @brief Esta função anima a aterragem de um fighter, ajustando sua direção, rotação e posição ao longo do tempo, 
 * de forma a regressar a uma posição neutra e virado para a saída do hangar em que aterrou. 
 * 
 * @param lightingShader Referência ao shader usado para iluminação na cena.
 * @param ponto O ponto de aterragem alvo.
 */
void animacaoAterragem(Shader& lightingShader, glm::vec3 ponto) {
    estacionado = 1;

    float endDirectionY = 0.0f;
    float endRotation = 0.0f;
    // Primeiramente ajustar a direção Y e rotação de modo a que o fighter não esteja inclinado em nenhuma direção
    while (fabs(fighter_player.directionY - endDirectionY) > 0.1f || fabs(fighter_player.rotation - endRotation) > 0.1f) {
        float t = deltaTime * 1.0f;
        fighter_player.directionY = glm::mix(fighter_player.directionY, endDirectionY, t);
        fighter_player.rotation = glm::mix(fighter_player.rotation, endRotation, t);
       
        renderScene(lightingShader);
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
      
        renderScene(lightingShader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Por fim mover o fighter para a posição desejada
    while (glm::distance(fighter_player.position, ponto) > 0.1f) {
        float t = deltaTime * 1.0f;
        fighter_player.position = glm::mix(fighter_player.position, ponto, t);

        renderScene(lightingShader);
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

    float movementSpeed = 50.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        fighter_player.position += movementSpeed * fighter_player.front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        fighter_player.position -= movementSpeed * fighter_player.front;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        fighter_player.rotation += deltaTime * 10.0f;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        fighter_player.rotation -= deltaTime * 10.0f;

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
    float sensitivity = 0.01f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    if(estacionado == 1)
        return;
        
    // Update da direção da camera
    camera.ProcessMouseMovement(xoffset, yoffset);
    // Update da direção e front do fighter_player
    fighter_player.front = camera.Front;
    fighter_player.directionX = camera.Yaw;
    fighter_player.directionY = camera.Pitch;
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
void renderScene(Shader& lightingShader) {
    // Clear the screen
    glClearColor(0.01f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update camera position
    glm::vec3 cameraOffset(0.0f, 0.0f, 60.0f);
    camera.Position = fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, 20.0f, 0.0f);
    camera.Front = glm::normalize(fighter_player.position - camera.Position);

    // Set shader uniforms for basic lighting
    lightingShader.use();
    lightingShader.setVec3("viewPos", camera.Position);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 700.0f);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);

    // Render hangars
    auto renderHangar = [&](glm::vec3 translation, float rotation, glm::vec3 lightPos, glm::vec3 lightColor) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("lightColor", lightColor);
        lightingShader.setMat4("model", model);
        lightingShader.setVec3("objectColor", 0.75f, 0.75f, 0.75f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    };
    renderHangar(glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, lightPos, blueLightColor);
    renderHangar(glm::vec3(500.0f, 0.0f, 0.0f), 180.0f, lightPos2, redLightColor);

    // Render fighter
    glm::mat4 model = glm::translate(glm::mat4(1.0f), fighter_player.position);
    model = glm::rotate(model, glm::radians(fighter_player.directionX + 180), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.directionY), glm::vec3(0.0f, 0.0f, -1.0f));
    model = glm::rotate(model, glm::radians(fighter_player.rotation), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(3.0f));

    // Determinar a fonte de luz mais próxima do fighter
    // Definir a iluminação do fighter
    lightingShader.setVec3("lightColor", blueLightColor);
    lightingShader.setVec3("lightPos", blueLightColor);
    lightingShader.setMat4("model", model);
    lightingShader.setVec3("objectColor", 0.3f, 0.3f, 0.3f);

    glBindVertexArray(VAO2);
    glDrawArrays(GL_TRIANGLES, 0, vertices2.size());
}

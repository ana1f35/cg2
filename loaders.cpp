#include "headers/loaders.h"

#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include FT_FREETYPE_H

/**
 * @brief Função utilizada para carrega um modelo 3D a partir de um ficheiro .obj usando a biblioteca TinyObjLoader 
 * para a leitura do ficheiro. São extraidos os vertices e as normais, e caso necessário centraliza o modelo.
 * Depois configura os objetos de array e buffer de vértices do OpenGL.
 * 
 * @param filePath Caminho para o ficheiro .obj a ser carregado.
 * @param model Referência para a estrutura Model onde será armazenado o modelo carregado.
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

    // Carregar o ficheiro .obj através do TinyObjLoader
    if (!tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &err, filePath.c_str(), mtlBaseDir.c_str())) {
        std::cout << "Failed to load OBJ file: " << err << std::endl;
        return -1;
    }

    glm::vec3 center(0.0f);
    int totalVertices = 0;

    // Extrair os vertices, normais e coordenadas de textura do ficheiro .obj
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

    // Centralizar o modelo
    if (centerModel) {
        center /= totalVertices;
        for (auto& vertex : model.vertices) {
            vertex -= center;
        }
    }

    // Configurar os objetos de array e buffer de vértices do OpenGL
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

    // Carregar os materiais do ficheiro .mtl
    if (!tinyMaterials.empty()) {
        std::string mtlFilePath = mtlBaseDir + model.name + ".mtl";
        std::cout << "Loading materials from: " << mtlFilePath << std::endl;
        loadMaterials(mtlFilePath, model.materials);
    } else {
        std::cout << "No materials found in the OBJ file." << std::endl;
    }

    return 0;
}

/**
 * @brief Função utilizada para carregar os materiais de um ficheiro .mtl e as respetivas texturas.
 * A função lê o ficheiro .mtl linha a linha e armazena as informações dos materiais num vetor de MaterialInfo.
 * 
 * @param mtlFilePath Caminho para o ficheiro .mtl a ser carregado.
 * @param materials Vetor de MaterialInfo onde serão armazenados os materiais carregados.
 */
void loadMaterials(const std::string& mtlFilePath, std::vector<MaterialInfo>& materials) {
    // Abrir o ficheiro .mtl
    std::ifstream mtlFile(mtlFilePath);
    if (!mtlFile) {
        std::cerr << "Failed to open MTL file: " << mtlFilePath << std::endl;
        return;
    }

    std::string line;
    MaterialInfo material;
    while (std::getline(mtlFile, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        // Processar cada linha do ficheiro .mtl
        if (prefix == "newmtl") {
            if (!material.name.empty()) {
                materials.push_back(material);
            }
            iss >> material.name;
            material.diffuse_texname = "";
            material.specular_texname = "";
            material.normal_texname = "";
            material.diffuse_texid = 0;
            material.specular_texid = 0;
            material.normal_texid = 0;
            material.ambient = glm::vec3(0.0f);
            material.diffuse = glm::vec3(0.0f);
            material.specular = glm::vec3(0.0f);
            material.shininess = 32.0f;
        } else if (prefix == "Kd") {
            iss >> material.diffuse.r >> material.diffuse.g >> material.diffuse.b;
        } else if (prefix == "Ka") {
            iss >> material.ambient.r >> material.ambient.g >> material.ambient.b;
        } else if (prefix == "Ks") {
            iss >> material.specular.r >> material.specular.g >> material.specular.b;
        } else if (prefix == "Ns") {
            iss >> material.shininess;
        } else if (prefix == "map_Kd") {
            iss >> material.diffuse_texname;
        } else if (prefix == "map_Ks") {
            iss >> material.specular_texname;
        } else if (prefix == "map_Bump" || prefix == "bump") {
            iss >> material.normal_texname;
        }
    }
    if (!material.name.empty()) {
        materials.push_back(material);
    }

    // Carregar as texturas dos materiais do ficheiro .mtl
    for (auto& material : materials) {
        if (!material.diffuse_texname.empty()) {
            std::string texPath = mtlFilePath.substr(0, mtlFilePath.find_last_of('/')) + "/" + material.diffuse_texname;
            material.diffuse_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded diffuse texture: " << texPath << std::endl;
        }
        if (!material.specular_texname.empty()) {
            std::string texPath = mtlFilePath.substr(0, mtlFilePath.find_last_of('/')) + "/" + material.specular_texname;
            material.specular_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded specular texture: " << texPath << std::endl;
        }
        if (!material.normal_texname.empty()) {
            std::string texPath = mtlFilePath.substr(0, mtlFilePath.find_last_of('/')) + "/" + material.normal_texname;
            material.normal_texid = loadTexture(texPath.c_str());
            std::cout << "Loaded normal texture: " << texPath << std::endl;
        }
    }
}


/**
 * @brief Função utilizada para carregar uma textura a partir de um ficheiro de imagem usando a biblioteca STB Image.
 * A função carrega a textura, gera um ID para a mesma e configura os parâmetros da textura.
 * 
 * @param path Caminho para o ficheiro de imagem a ser carregado.
 * @return unsigned int - Retorna o ID da textura carregada.
 */
unsigned int loadTexture(const char* path) {
    //rotação da textura para que a imagem seja carregada corretamente
    stbi_set_flip_vertically_on_load(true);

    //Aloca um ID para a textura
    unsigned int textureID;
    glGenTextures(1, &textureID);

    //Carrega a textura
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

        //Define os parâmetros da textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Liberta a memória da textura
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


/**
 * @brief Função utilizada para carregar um cubo de luz para ser usado na cena.
 */
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

    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

/**
 * @brief Função utilizada para carregar texto a partir de um ficheiro de fonte TrueType.
 * 
 * @return int - Retorna 0 se foi bem sucedido, ou -1 se ocorreu erro ao carregar o ficheiro.
 */
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
 * @brief Função que carrega as varias texturas para o skybox.
 * A função carrega as 6 imagens para o criar o skybox, cria um cubemap e configura os parâmetros da textura.
 * 
 * @param faces Vetor de strings com os caminhos para as texturas do skybox.
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
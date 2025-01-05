#include "include/setups.h"

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

void setupSquare() {
    float vertices[] = {
        // positions        // texture coords
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,

        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &exVAO);
    glGenBuffers(1, &exVBO);

    glBindVertexArray(exVAO);

    glBindBuffer(GL_ARRAY_BUFFER, exVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
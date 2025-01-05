#include "headers/input.h"

/**
 * @brief Esta função verifica inputs de teclas específicas e atualiza a posição e rotação do fighter com base no input. 
 * Também lida com o evento de fecho da janela quando a tecla ESC é pressionada.
 * 
 * Correspondência das teclas:
 * ESC: Fecha a janela.
 * W: Incrementa a velocidade. Incrementa o valor de fighter_player.movementSpeed.
 * S: Decrementa a velocidade. Decrementa o valor de fighter_player.movementSpeed.
 * Q: Ataca.
 * V: Alteração da perspetiva de câmera.
 * C: Pausa o jogo e mostra os controlos do jogo.
 */
void processInput()
{
    // Ao clicar no ESC fecha a janela
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // caso esteja a ser passada a introdução é possível saltar clicando em E
    if(gameState == 5 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gameState = 0;

    // caso o jogo não tenha iniciado, esteja em pausa ou no estado de introdução não devem ser lidos mais inputs
    if(gameState == 0 || gameState == 3 || gameState == 5)
        return;

    // caso tenha terminado o jogo deverá ser possível recomeçar clicando no SPACE
    if(gameState == 4){
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            restartGame();
            gameState = 1;
        }
        return;
    }

    // Durante o jogo, ao clicar no C deverá pausar e mostrar os controlos do jogo, ou o contrário.
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

    // Altera entre os 3 modos de câmera disponíveis
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

    // Ao clicar Q será reproduzido um som e chamada a função que permite ao jogador atacar uma nave inimiga
    static bool fireKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (!fireKeyPressed) {
            alSourcePlay(source2);
            shootProjectile(fighter_player, 0);
            fireKeyPressed = true;
        }
        } else {
            fireKeyPressed = false;
        }

    // Estando no modo de camera de visão superior, a nave deverá ser controlado pelas teclas A e D movendo para a esquerda e direita
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

    // Aumenta a velocidade da nave e reproduz um som
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (fighter_player.movementSpeed < 100.0f) {
            fighter_player.movementSpeed += 50.0f;
            alSourcePlay(source3);
        }
        lastPressTime = glfwGetTime();
    }
    // Diminui a velocidade
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (fighter_player.movementSpeed > 30.0f) {
            fighter_player.movementSpeed -= 1.0f;
        } else {
            fighter_player.movementSpeed = 30.0f; // Velocidade minima
        }
    }
    else {
        // A velocidade deve ir diminuindo gradualmente
        float currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 1.0f) {
            if (fighter_player.movementSpeed > 30.0f) {
                fighter_player.movementSpeed -= 5.0f; 
            } else {
                fighter_player.movementSpeed = 30.0f;
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

    // Atualiza a direção da camera com base no input do rato
    camera.ProcessMouseMovement(xoffset, yoffset);
    // Limita a direção dentro deste intervalo de ângulos
    if (camera.Yaw > 50.0f)
        camera.Yaw = 50.0f;
    if (camera.Yaw < -50.0f)
        camera.Yaw = -50.0f;

    // Atualiza a direção do jogador com base na camera
    fighter_player.front = camera.Front;
    fighter_player.directionX = camera.Yaw;
    fighter_player.directionY = camera.Pitch;

    // Suavemente vai inclinando a nave quando esta muda de direção
    const float rotationSpeed = 5.0f;
    fighter_player.rotation -= xoffset * rotationSpeed * 0.05;
    if (fighter_player.rotation > 45.0f) {
        fighter_player.rotation = 45.0f;
    } else if (fighter_player.rotation < -45.0f) {
        fighter_player.rotation = -45.0f;
    }

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

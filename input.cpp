#include "include/input.h"

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
            shootProjectile(fighter_player, 0);
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

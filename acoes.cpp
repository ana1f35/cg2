#include "headers/acoes.h"


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

void animacaoInimigos(){
    float targetY = 40.0f;
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
        std::string pontuacaoStr = "Score: " + std::to_string(pontuacao);
        RenderText(vida, SCR_WIDTH - 300.0f, SCR_HEIGHT - 100.0f, 1.0f, textColor, true);
        RenderText(pontuacaoStr, SCR_WIDTH - 300.0f, SCR_HEIGHT - 180.0f, 1.0f, textColor, true);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

bool isColliding(glm::vec3 projPosition, float projRadius, glm::vec3 enemyPosition, float enemyRadius) {
    float distance = glm::distance(projPosition, enemyPosition);
    std::cout << "Distance between projectile and enemy: " << distance << std::endl;
    std::cout << "Sum of radii: " << (projRadius + enemyRadius) << std::endl;
    return distance < (projRadius + enemyRadius);
}

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

                if (isColliding(projIt->position, projIt->collisionRadius, enemyIt->position, enemyIt->collisionRadius)) {
                    std::cout << "Collision detected between projectile and enemy" << std::endl;
                    enemyIt->hp -= 1; // Reduz a vida do inimigo
                    std::cout << "Enemy HP: " << enemyIt->hp << std::endl;

                    if (enemyIt->hp <= 0) {
                        std::cout << "Enemy destroyed" << std::endl;
                        explosions.push_back({enemyIt->position, (float) glfwGetTime()});
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

void shootProjectile(const Fighter& fighter, int origin) {
    glm::vec3 projectilePosition = fighter.position + fighter.front * 5.0f;
    glm::vec3 direction = fighter.front;

    // Encontre o inimigo mais próximo
    if (origin == 0) { // Verifica se a origem do tiro é do jogador principal
        Fighter* closestEnemy = findClosestEnemy(fighter.position);
        if (closestEnemy) {
            float distance = glm::distance(fighter.position, closestEnemy->position);
            float maxAutoAimDistance = 700.0f; // Aumente a distância máxima para a mira automática

            if (distance < maxAutoAimDistance) {
                direction = glm::normalize(closestEnemy->position - fighter.position);
            }
        }
    }

    float projRadius = 2.0f; // Defina o raio de colisão para o projétil
    projectiles.emplace_back(projectilePosition, direction, 20.0f, projRadius, origin); // Velocidade ajustada para 20.0f
}


void updateProjectiles(float deltaTime) {
    for (auto& proj : projectiles) {
        proj.position += proj.direction * proj.speed * deltaTime * 0.5f;
        // std::cout << "Projectile updated to position: " << proj.position.x << ", " << proj.position.y << ", " << proj.position.z << std::endl;
    }

    // Remover projéteis que saíram dos limites
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& proj) {
        return proj.position.z > 600.0f || proj.position.z < -600.0f; // Limites arbitrários
    }), projectiles.end());

    // std::cout << "Projectiles updated. Count: " << projectiles.size() << std::endl;
}

void shootEnemyProjectiles(float deltaTime) {
    static float lastShootTime = 0.0f;
    float currentTime = glfwGetTime();

    // Atira a cada 3 segundos
    if (currentTime - lastShootTime >= 3.0f) {
        for (const auto& enemy : enemies) {
            // Verificar a distância entre o inimigo e o jogador
            float distanceToPlayer = glm::distance(enemy.position, fighter_player.position);
            float maxShootingDistance = 600.0f; // Defina a distância máxima para atirar

            if (distanceToPlayer <= maxShootingDistance) {
                glm::vec3 projectilePosition = enemy.position + enemy.front * 5.0f; // Posição inicial
                glm::vec3 projectileDirection = glm::normalize(fighter_player.position - enemy.position);

                // Introduzir uma variação aleatória na direção do projétil
                float variation = 0.05f; // Ajuste conforme necessário
                projectileDirection.x += ((rand() % 100) / 100.0f - 0.5f) * variation;
                projectileDirection.y += ((rand() % 100) / 100.0f - 0.5f) * variation;
                projectileDirection.z += ((rand() % 100) / 100.0f - 0.5f) * variation;
                projectileDirection = glm::normalize(projectileDirection); // Normalizar a direção

                // Velocidade ajustada para projéteis inimigos
                float projectileSpeed = 20.0f; // Ajuste conforme necessário
                projectiles.emplace_back(projectilePosition, projectileDirection, projectileSpeed, 2.0f, 1);

                std::cout << "Enemy projectile shot from position: " << projectilePosition.x << ", " << projectilePosition.y << ", " << projectilePosition.z << std::endl;
            }
        }
        lastShootTime = currentTime;
    }
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

void restartGame(){
    cameraMode = 0;
    pontuacao = 0;
    
    camera = glm::vec3(0.0f, 0.0f, 0.0f);
    fighter_player = Fighter(glm::vec3(43.2f, 54.0f, -33.0f), camera.Front, 0.0f, camera.Yaw, camera.Pitch, 30.0f, 3, 10.0f);

    enemies.clear();
    enemies = {
        Fighter(glm::vec3(750.0f, 50.0f, -90.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
        Fighter(glm::vec3(600.0f, 20.0f, 0.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),
        Fighter(glm::vec3(800.0f, 50.0f, 90.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 4.0f, 1, 10.0f),

        Fighter(glm::vec3(2000.0f, 100.0f, -200.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f),
        Fighter(glm::vec3(2100.0f, 100.0f, 200.0f), -fighter_player.front, 0.0f, camera.Yaw, camera.Pitch, 5.0f, 1, 10.0f)
    };

    projectiles.clear();

    animacaoSaida();
}
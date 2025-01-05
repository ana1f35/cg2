#include "headers/acoes.h"

/**
 * @brief Função que move automaticamente as naves inimigas em direção ao jogador.
 */
void moverInimigos() {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        // mover em frente
        glm::vec3 finalPos = glm::vec3(0.0f, fighter_player.position.y, 0.0f);
        glm::vec3 direction = glm::normalize(finalPos - it->position);
        glm::vec3 newPosition = it->position + direction * it->movementSpeed * 0.5f;
        it->position = newPosition;

        // Verificar colisão com a nave principal, caso aconteça a nave inimiga perde um ponto de vida
        // sendo destruída, e a nave principal perde um ponto de vida
        if (isColliding(it->position, it->collisionRadius, fighter_player.position, fighter_player.collisionRadius)) {
            fighter_player.hp -= 1; // Reduzir 1 HP da nave principal
            it = enemies.erase(it); // Destruir a nave inimiga
            continue;
        }

        // Verificar se o inimigo está fora do ângulo de visão do player
        // caso esteja deverá ser apagado de cena
        glm::vec3 toEnemy = glm::normalize(it->position - fighter_player.position);
        float dotProduct = glm::dot(fighter_player.front, toEnemy);
        float angleToPlayer = glm::degrees(glm::acos(dotProduct));
        if (angleToPlayer > 60.0f) {
            it = enemies.erase(it);
            continue;
        }

        // rodar a nave inimiga na direção correta: em direção ao jogador
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
 * @brief Esta função verifica se o jogo está no estado inicial e caso esteja e a tecla de espaço for pressionada, é acionada a animação de partida.
 *
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
 * @brief Esta função anima a posição vertical da nave da sua posição atual para uma altura de 20.0 unidades. 
 * Durante a animação, a cena é continuamente renderizada e os buffers da janela são trocados para exibir a posição atualizada.
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

/**
 * @brief Esta função anima a posição vertical da nave da sua posição atual para uma altura de 40.0 unidades. 
 * Durante a animação, a cena é continuamente renderizada e os buffers da janela são trocados para exibir a posição atualizada.
 */
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

/**
 * @brief Verifica se há uma colisão entre um projétil e um inimigo.
 * 
 * Esta função determina se um projétil colidiu com um inimigo, calculando a distância
 * entre suas posições e comparando essa distância com a soma de seus raios de colisão.
 * 
 * A colisão é considerada ocorrida se a distância entre o centro do projétil e o centro
 * do inimigo for menor que a soma dos raios de colisão de ambos, ou seja, a hitbox dos 
 * dois objetos se sobrepõem ou se encostam.
 * 
 * @param projPosition Posição do projétil.
 * @param projRadius Raio de colisão do projétil.
 * @param enemyPosition Posição do inimigo.
 * @param enemyRadius Raio de colisão do inimigo.
 * @return true Se houve colisão.
 * @return false Se não houve colisão.
 */

bool isColliding(glm::vec3 projPosition, float projRadius, glm::vec3 enemyPosition, float enemyRadius) {
    // Calcula a distância entre o projétil e o inimigo
    float distance = glm::distance(projPosition, enemyPosition);
    
    // Print para Debug
    std::cout << "Distance between projectile and enemy: " << distance << std::endl;
    std::cout << "Sum of radii: " << (projRadius + enemyRadius) << std::endl;
    
    // Verifica se a distância é menor que a soma dos raios de colisão
    return distance < (projRadius + enemyRadius);
}

/**
 * @brief Verifica colisões entre projéteis e inimigos ou o jogador.
 * 
 * Esta função percorre a lista de projéteis e verifica se há colisões com inimigos
 * (se o projétil for do jogador) ou com o jogador (se o projétil for de um inimigo).
 * Se uma colisão for detectada, a vida do inimigo ou do jogador é reduzida e o projétil
 * é removido. Se a vida do inimigo chegar a zero, ele é destruído e removido da lista.
 */

void checkCollisions() {
    // Percorre todos os projéteis
    for (auto projIt = projectiles.begin(); projIt != projectiles.end();) {
        bool collisionDetected = false;

        // Verifica colisões com inimigos
        if (projIt->origin == 0) {   // Projétil do jogador
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                // Print para Debug
                std::cout << "Checking collision between projectile at (" 
                          << projIt->position.x << ", " << projIt->position.y << ", " << projIt->position.z 
                          << ") with radius " << projIt->collisionRadius 
                          << " and enemy at (" 
                          << enemyIt->position.x << ", " << enemyIt->position.y << ", " << enemyIt->position.z 
                          << ") with radius " << enemyIt->collisionRadius << std::endl;

                // Verifica se há colisão entre o projétil e o inimigo
                if (isColliding(projIt->position, projIt->collisionRadius, enemyIt->position, enemyIt->collisionRadius)) {
                    std::cout << "Collision detected between projectile and enemy" << std::endl;
                    enemyIt->hp -= 1; // Reduz a vida do inimigo
                    std::cout << "Enemy HP: " << enemyIt->hp << std::endl;

                    // Se a vida do inimigo chegar a zero, ele é destruído
                    if (enemyIt->hp <= 0) {
                        std::cout << "Enemy destroyed" << std::endl;
                        explosions.push_back({enemyIt->position, (float) glfwGetTime()});
                        pontuacao += 10; // Aumenta o Score
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
            // Verifica se há colisão entre o projétil e o jogador
            if (isColliding(projIt->position, projIt->collisionRadius, fighter_player.position, fighter_player.collisionRadius)) {
                std::cout << "Collision detected between projectile and player" << std::endl;
                fighter_player.hp -= 1; // Reduz a vida do jogador
                collisionDetected = true;
            }
        }

        // Se uma colisão foi detectada, remove o projétil
        if (collisionDetected) {
            std::cout << "Projectile removed" << std::endl;
            projIt = projectiles.erase(projIt); // Remove o projétil que colidiu
        } else {
            ++projIt; // Move para o próximo projétil
        }
    }
}

/**
 * @brief Dispara um projétil a partir da posição de um lutador (fighter).
 * 
 * Esta função cria um novo projétil na posição do fighter e o adiciona à lista de projéteis.
 * Se o projétil for disparado pelo jogador, ele tentará mirar no inimigo mais próximo.
 * 
 * @param fighter Referência ao lutador que está disparando o projétil.
 * @param origin Identificador da origem do projétil (0 para jogador, 1 para inimigo).
 */
void shootProjectile(const Fighter& fighter, int origin) {
    // Define a posição inicial do projétil um pouco à frente do lutador
    glm::vec3 projectilePosition = fighter.position + fighter.front * 5.0f;
    glm::vec3 direction = fighter.front;

    // Se o projétil for disparado pelo jogador, tenta mirar no inimigo mais próximo
    if (origin == 0) { // Verifica se a origem do tiro é do jogador principal
        Fighter* closestEnemy = findClosestEnemy(fighter.position);
        if (closestEnemy) {
            float distance = glm::distance(fighter.position, closestEnemy->position);
            float maxAutoAimDistance = 700.0f; // Distância máxima para a mira automática

            // Se o inimigo mais próximo estiver dentro da distância máxima, ajusta a direção do projétil
            if (distance < maxAutoAimDistance) {
                direction = glm::normalize(closestEnemy->position - fighter.position);
            }
        }
    }

    // Define o raio de colisão para o projétil
    float projRadius = 5.0f;
    // Adiciona o novo projétil à lista de projéteis
    projectiles.emplace_back(projectilePosition, direction, 20.0f, projRadius, origin); // Velocidade ajustada para 20.0f
}

/**
 * @brief Atualiza a posição de todos os projéteis na cena.
 * 
 * Esta função percorre a lista de projéteis e atualiza a posição de cada um deles com base na sua direção e velocidade.
 * Se um projétil sair dos limites definidos da cena, ele é removido da lista de projéteis.
 * 
 * @param deltaTime Tempo decorrido desde a última atualização, usado para calcular o deslocamento do projétil.
 */
void updateProjectiles(float deltaTime) {
    for (auto& proj : projectiles) {
        // Atualiza a posição do projétil com base na sua direção e velocidade
        proj.position += proj.direction * proj.speed * deltaTime * 0.5f;
        // std::cout << "Projectile updated to position: " << proj.position.x << ", " << proj.position.y << ", " << proj.position.z << std::endl;
    }

    // Remover projéteis que saíram dos limites
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& proj) {
        return proj.position.z > 600.0f || proj.position.z < -600.0f; // Limites arbitrários
    }), projectiles.end());

    // std::cout << "Projectiles updated. Count: " << projectiles.size() << std::endl;
}


/**
 * @brief Dispara projéteis de todos os inimigos em direção ao jogador.
 * 
 * Esta função percorre a lista de inimigos e faz com que cada inimigo dispare um projétil
 * em direção à posição atual do jogador. Os projéteis disparados são adicionados à lista
 * de projéteis.
 * 
 * @param deltaTime Tempo decorrido desde a última atualização, usado para calcular o deslocamento do projétil.
 *
 */
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


/**
 * @brief Cria a geometria de um projétil cilíndrico e configura os buffers de vértices.
 * 
 * Esta função gera os vértices para um projétil cilíndrico com tampas superior e inferior,
 * e configura os buffers de vértices (VBO) e o array de vértices (VAO) para renderização.
 * 
 * @param radius Raio do cilindro do projétil.
 * @param height Altura do cilindro do projétil.
 * @param segments Número de segmentos para definir a circunferência do cilindro.
 * @param VAO Referência para o identificador do Vertex Array Object.
 * @param VBO Referência para o identificador do Vertex Buffer Object.
 */
void createShot(float radius, float height, int segments, unsigned int& VAO, unsigned int& VBO){
    std::vector<float> shotVertices;

    // Gerar vértices para o topo e a base do cilindro
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

/**
 * @brief Encontra o inimigo mais próximo de uma posição especificada.
 * 
 * Esta função percorre a lista de inimigos e encontra o inimigo cuja posição está mais próxima
 * da posição especificada. Retorna um ponteiro para o inimigo mais próximo ou nullptr se não houver inimigos.
 * 
 * @param playerPosition Posição de referência para encontrar o inimigo mais próximo.
 * @return Fighter* Ponteiro para o inimigo mais próximo ou nullptr se não houver inimigos.
 */
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

/**
 * @brief Esta permite reiniciar os valores das variaveis globais permitindo reiniciar o jogo sempre que este termina.
 */
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


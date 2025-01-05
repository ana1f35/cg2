#ifndef ACOES_H
#define ACOES_H

#include "common.h"
#include "render.h"
#include "input.h"
#include "loaders.h"
#include "structs.h"
#include "common.h"

extern std::vector<Fighter> enemies;
extern Fighter fighter_player;
extern std::vector<Projectile> projectiles;
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;
extern glm::vec3 textColor;
extern int pontuacao;
extern GLFWwindow* window;

bool checkStart();
void animacaoSaida();
void moverInimigos();
void animacaoInimigos();
bool isColliding(glm::vec3 projPosition, float projRadius, glm::vec3 enemyPosition, float enemyRadius);
void checkCollisions();
void shootProjectile(const Fighter& fighter, int origin);
void updateProjectiles(float deltaTime);
void shootEnemyProjectiles(float deltaTime);
void createShot(float radius, float height, int segments, unsigned int& VAO, unsigned int& VBO);
Fighter* findClosestEnemy(const glm::vec3& playerPosition);
void restartGame();

#endif
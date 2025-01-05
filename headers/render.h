#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "structs.h"
#include "loaders.h"
#include "audio.h"
#include "input.h"
#include "setups.h"

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;
extern unsigned int cameraMode;
extern unsigned int gameState;
extern int pontuacao;
extern glm::vec3 hangarPos;
extern glm::vec3 enemyHangarPos;
extern glm::vec3 spotLightPositions[9];
extern glm::vec3 pointLightPositions[7];
extern glm::vec3 starLightColor;
extern glm::vec3 pointStarLightColor;
extern glm::vec3 redLightColor;
extern glm::vec3 blueLightColor;
extern Camera camera;
extern Fighter fighter_player;
extern std::vector<Fighter> enemies;
extern std::vector<Explosion> explosions;
extern unsigned int explosionTextureID;
extern unsigned int exVAO;
extern Shader* skyboxShader;
extern Shader* lightingShader;
extern Shader* lightingCubeShader;
extern Shader* textShader;
extern Shader* hitBoxShader;
extern Shader* explosionShader;
extern Shader* lightingPTexShader;
extern unsigned int skyboxVAO;
extern unsigned int cubemapTexture;
extern std::map<GLchar, Character> Characters;
extern std::map<GLchar, Character> Characters2;
extern unsigned int VAOt;
extern unsigned int VBOt;
extern std::vector<Projectile> projectiles;
extern unsigned int VAOProjectile;
extern unsigned int numProjectileVertices;
extern Model xwingModel;
extern Model hangarModel;
extern Model tieModel;
extern glm::vec3 textColor;

void renderScene();
void renderTextures(Model *model);
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, bool useFirstFont);
void renderBoundingBox(glm::vec3 position, float radius, float scaleFactor);
void renderProjectiles(Shader& shader);
void renderIntro();
void desenhaAlvo();
void lightsActivate(Shader *ls, glm::mat4 view, glm::mat4 projection);

#endif
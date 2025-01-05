#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "setups.h"
#include "structs.h"
#include "loaders.h"
#include "audio.h"

extern GLFWwindow* window;
extern unsigned int gameState;
extern unsigned int cameraMode;
extern GLuint source2, source3;
extern Fighter fighter_player;
extern void restartGame();
extern void shootProjectile(const Fighter& fighter, int origin);
extern Camera camera;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput();

#endif
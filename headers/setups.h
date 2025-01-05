#ifndef SETUPS_H
#define SETUPS_H

#include "common.h"

extern unsigned int skyboxVAO, skyboxVBO;
extern unsigned int exVAO, exVBO;
extern unsigned int boundingBoxVAO, boundingBoxVBO, boundingBoxEBO;

void setupSkybox();
void setupSquare();
void setupBoundingBox();

#endif
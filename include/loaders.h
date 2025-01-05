#ifndef LOADERS_H
#define LOADERS_H

#include "common.h"
#include "structs.h"

extern unsigned int lightCubeVAO;
extern unsigned int lightVBO;
extern unsigned int VAOt;
extern unsigned int VBOt;
extern std::map<GLchar, Character> Characters;
extern std::map<GLchar, Character> Characters2;

int loadModel(const std::string& filePath, Model& model, bool centerModel);
void loadMaterials(const std::string& mtlFilePath, std::vector<MaterialInfo>& materials);
unsigned int loadTexture(const char* path);
void loadLuz();
int loadText();

#endif
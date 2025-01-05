#ifndef STRUCTS_H
#define STRUCTS_H

struct MaterialInfo {
    std::string name;
    std::string diffuse_texname;
    std::string specular_texname;
    std::string normal_texname;
    unsigned int diffuse_texid;
    unsigned int specular_texid;
    unsigned int normal_texid;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

struct Model {
    std::string name;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    unsigned int VAO, VBO, NBO, TBO;
    std::vector<MaterialInfo> materials;
};

struct Fighter {
    glm::vec3 position;
    glm::vec3 front;
    float rotation;
    float directionX;
    float directionY;
    float movementSpeed;
    int hp;
    float collisionRadius;
        
    Fighter(glm::vec3 pos, glm::vec3 frnt, float rotation, float directionX, float directionY, float speed, int hp, float radius) 
        : position(pos), front(frnt), rotation(rotation), directionX(directionX), directionY(directionY), movementSpeed(speed), hp(hp), collisionRadius(radius) {}
};

struct Projectile {
    glm::vec3 position;
    glm::vec3 direction;
    float speed;
    float collisionRadius;
    int origin; // 0 = player, 1 = enemy

    Projectile(glm::vec3 pos, glm::vec3 dir, float spd, float radius, int origin)
        : position(pos), direction(glm::normalize(dir)), speed(spd), collisionRadius(radius), origin(origin) {}
};

struct Character {
    unsigned int TextureID; 
    glm::ivec2   Size;      
    glm::ivec2   Bearing;   
    unsigned int Advance;   
};

struct Explosion {
    glm::vec3 position;
    float startTime;
};

#endif
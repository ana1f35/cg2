#include "headers/render.h"

/**
 * @brief Esta função renderiza a cena principal do jogo com iluminação e os objetos.
 * Começa por limpar a tela, passando depois a movimentar automaticamente o jogador para a frente.
 * Dependendo do modo de câmera ativado é configurada a posição e ângulos da câmera.
 */
void renderScene() {
    // Limpar a tela
    glClearColor(0.01f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Movimento automatico em direção ao hangar inimigo
    if(gameState == 1){
        glm::vec3 direction = glm::vec3(
            cos(glm::radians(fighter_player.directionY)) * cos(glm::radians(fighter_player.directionX)),
            sin(glm::radians(fighter_player.directionY)),
            cos(glm::radians(fighter_player.directionY)) * sin(glm::radians(fighter_player.directionX))
        );
        fighter_player.position += fighter_player.movementSpeed * direction * 0.05f;

        // Gradualmente repor a inclinação da nave quando para de virar
        if (fighter_player.rotation > 0.0f) {
            fighter_player.rotation -= 0.5f; 
            if (fighter_player.rotation < 0.0f) {
                fighter_player.rotation = 0.0f;
            }
        } else if (fighter_player.rotation < 0.0f) {
            fighter_player.rotation += 0.5f; 
            if (fighter_player.rotation > 0.0f) {
                fighter_player.rotation = 0.0f;
            }
        }
    }

    // Atualizar a posição e ângulos da câmera dependendo do modo escolhido
    if(cameraMode == 0){
        glm::vec3 cameraOffset(0.0f, 10.0f, 60.0f);
        glm::vec3 finalPos = glm::vec3(fighter_player.position - fighter_player.front * cameraOffset.z + glm::vec3(0.0f, cameraOffset.y, 0.0f));
        if (glm::distance(camera.Position, finalPos) < 5)
            camera.Position = glm::mix(camera.Position, finalPos, 0.5f);
        else
            camera.Position = glm::mix(camera.Position, finalPos, 0.1f);
    }
    else if (cameraMode == 1) {
        camera.Position = fighter_player.position;
    }
    else {
        glm::vec3 cameraOffset(100.0f, 300.0f, 0.0f); 
        camera.Position = fighter_player.position + cameraOffset;
        camera.Front = glm::vec3(0.0f, -1.0f, 0.0f);
        camera.Up = glm::vec3(1.0f, 0.0f, 0.0f); 
        fighter_player.directionX = 0.0f;
        fighter_player.directionY = 0.0f;
        fighter_player.rotation = 0.0f;
    }

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    lightingShader->use();


    // Ativar o shader com texturas e definir os seus uniformes 
    lightsActivate(lightingPTexShader, view, projection);

    glm::mat4 model = glm::mat4(1.0f);

    // Render dos inimigos 
    for(Fighter enemy : enemies){
        model = glm::translate(glm::mat4(1.0f), enemy.position);
        model = glm::rotate(model, glm::radians(enemy.directionX + 90), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(enemy.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(3.0f));
        lightingPTexShader->setFloat("material.shininess", 32.0f);
        lightingPTexShader->setMat4("model", model);
        renderTextures(&xwingModel); //renderiza o modelo do inimigo com texturas
    }

    // Activar o shader basico e definir os seus uniformes
    lightsActivate(lightingShader, view, projection);

    // Render dos hangars
    auto renderHangar = [&](glm::vec3 translation, float rotation) {
        model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        for (MaterialInfo material : hangarModel.materials) {
            lightingShader->setVec3("material.ambient", material.ambient);
            lightingShader->setVec3("material.diffuse", material.diffuse);
            lightingShader->setVec3("material.specular", material.specular);
            lightingShader->setFloat("material.shininess", material.shininess);
            lightingShader->setMat4("model", model);
            glBindVertexArray(hangarModel.VAO);
            glDrawArrays(GL_TRIANGLES, 0, hangarModel.vertices.size());
        }
    };
    renderHangar(hangarPos, 0.0f);
    renderHangar(enemyHangarPos, 180.0f);

    // Render da nave do jogador
    model = glm::translate(glm::mat4(1.0f), fighter_player.position);
    model = glm::rotate(model, glm::radians(fighter_player.directionX + 270), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.directionY), glm::vec3(-1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(fighter_player.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    model = glm::scale(model, glm::vec3(35.0f));
    for(MaterialInfo material : tieModel.materials){
        lightingShader->setVec3("material.ambient", material.ambient);
        lightingShader->setVec3("material.diffuse", material.diffuse);
        lightingShader->setVec3("material.specular", material.specular);
        lightingShader->setFloat("material.shininess", material.shininess);
        lightingShader->setMat4("model", model);
        glBindVertexArray(tieModel.VAO);
        glDrawArrays(GL_TRIANGLES, 0, tieModel.vertices.size());
    }

    // Render das lampadas dos hangars
    lightingCubeShader->use();
    lightingCubeShader->setMat4("projection", projection);
    lightingCubeShader->setMat4("view", view);
    for(glm::vec3 pos : spotLightPositions){
        model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::scale(model, glm::vec3(2.0f));
        lightingCubeShader->setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    for(glm::vec3 pos : spotLightPositions){
        model = glm::translate(glm::mat4(1.0f), (pos + enemyHangarPos + glm::vec3(20.0f, 0.0f, 0.0f)));
        model = glm::scale(model, glm::vec3(2.0f));
        lightingCubeShader->setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Render do skybox
    glDepthFunc(GL_LEQUAL); 
    skyboxShader->use();
    view = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
    skyboxShader->setMat4("view", view);
    skyboxShader->setMat4("projection", projection);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthFunc(GL_LESS); 

    // Render das explosões
    explosionShader->use();
    explosionShader->setMat4("view", camera.GetViewMatrix());
    explosionShader->setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f));
    explosionShader->setInt("explosionTexture", 0);

    float currentTime = glfwGetTime();
    for (auto it = explosions.begin(); it != explosions.end();) {
        //remove as explosões que já duram mais de 0.3 segundos
        if (currentTime - it->startTime > 0.3f) {
            // std::cout << "Explosion removed" << std::endl;
            it = explosions.erase(it);
        } else {
            // std::cout << "Explosion rendered" << std::endl;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), it->position);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(25.0f));
            explosionShader->setMat4("model", model);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, explosionTextureID);

            glBindVertexArray(exVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            ++it;
        }
    }

    glm::vec3 position = fighter_player.position;
    float radius = fighter_player.collisionRadius;

    renderBoundingBox(position, radius, 1.0);

    // Renderizar bounding boxes para os inimigos
    for (const auto& enemy : enemies) {
        renderBoundingBox(enemy.position, enemy.collisionRadius, 1.2);
    }

    desenhaAlvo();
}


/**
 * @brief Esta funcção renderiza as texturas de um modelo.
 * Começa por ligar o VAO do modelo e verificar se este tem materiais. Se tiver, aplica as texturas de acordo com o material.
 * Se não tiver, usa valores por defeito. No final, desenha o modelo.
 * 
 * @param Tmodel Ponteiro para o modelo a ser renderizado.
 */
void renderTextures(Model *Tmodel){
    glBindVertexArray(Tmodel->VAO);
    if (!Tmodel->materials.empty()) {
        bool anyMaterialApplied = false;
        for (const auto& material : Tmodel->materials){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, 0);
            if (!material.diffuse_texname.empty() || !material.specular_texname.empty() || !material.normal_texname.empty()) {
                anyMaterialApplied = true;
                glActiveTexture(GL_TEXTURE0);
                if (!material.diffuse_texname.empty()) {
                    glBindTexture(GL_TEXTURE_2D, material.diffuse_texid);
                    lightingPTexShader->setBool("hasDiffuseTexture", true);
                } else {
                    lightingPTexShader->setBool("hasDiffuseTexture", false);
                }

                glActiveTexture(GL_TEXTURE1);
                if (!material.specular_texname.empty()) {
                    glBindTexture(GL_TEXTURE_2D, material.specular_texid);
                    lightingPTexShader->setBool("hasSpecularTexture", true);
                } else {
                    lightingPTexShader->setBool("hasSpecularTexture", false);
                }

                glActiveTexture(GL_TEXTURE2);
                if (!material.normal_texname.empty()) {
                    glBindTexture(GL_TEXTURE_2D, material.normal_texid);
                    lightingPTexShader->setBool("hasNormalTexture", true);
                } else {
                    lightingPTexShader->setBool("hasNormalTexture", false);
                }
            }
        }
        if (!anyMaterialApplied) {
            lightingPTexShader->setBool("hasDiffuseTexture", false);
            lightingPTexShader->setBool("hasSpecularTexture", false);
            lightingPTexShader->setBool("hasNormalTexture", false);
            lightingPTexShader->setVec3("objectColor", 0.3f, 0.3f, 0.3f);
            glDrawArrays(GL_TRIANGLES, 0, Tmodel->vertices.size());
        }
    } else {
        lightingPTexShader->setBool("hasDiffuseTexture", false);
        lightingPTexShader->setBool("hasSpecularTexture", false);
        lightingPTexShader->setBool("hasNormalTexture", false);
        lightingPTexShader->setVec3("objectColor", 0.3f, 0.3f, 0.3f);
        std::cout << "No materials found for model" << std::endl;
    }

    glDrawArrays(GL_TRIANGLES, 0, Tmodel->vertices.size());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
}


/**
 * @brief Esta função renderiza as luzes da cena.
 */
void lightsActivate(Shader *ls, glm::mat4 view, glm::mat4 projection){
    ls->use();
    ls->setMat4("projection", projection);
    ls->setMat4("view", view);
    ls->setVec3("viewPos", camera.Position);

    // directional light
    ls->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    ls->setVec3("dirLight.ambient", starLightColor * glm::vec3(0.1f));
    ls->setVec3("dirLight.diffuse", starLightColor * glm::vec3(0.5f));
    ls->setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);

    // point light 1
    ls->setVec3("pointLights[0].position", pointLightPositions[0]);
    ls->setVec3("pointLights[0].ambient", blueLightColor * glm::vec3(0.3f));
    ls->setVec3("pointLights[0].diffuse", blueLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[0].specular", 0.6f, 0.6f, 0.6f);
    ls->setFloat("pointLights[0].constant", 1.0f);
    ls->setFloat("pointLights[0].linear", 0.002f);
    ls->setFloat("pointLights[0].quadratic", 0.0001f);
    // point light 2
    ls->setVec3("pointLights[1].position", pointLightPositions[1]);
    ls->setVec3("pointLights[1].ambient", redLightColor * glm::vec3(0.3f));
    ls->setVec3("pointLights[1].diffuse", redLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[1].specular", 0.6f, 0.6f, 0.6f);
    ls->setFloat("pointLights[1].constant", 1.0f);
    ls->setFloat("pointLights[1].linear", 0.002f);
    ls->setFloat("pointLights[1].quadratic", 0.0001f);
    // point light 3
    ls->setVec3("pointLights[2].position", pointLightPositions[2]);
    ls->setVec3("pointLights[2].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[2].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[2].specular", 0.8f, 0.8f, 0.8f);
    ls->setFloat("pointLights[2].constant", 1.0f);
    ls->setFloat("pointLights[2].linear", 0.014f);
    ls->setFloat("pointLights[2].quadratic", 0.0007f);
    // point light 4
    ls->setVec3("pointLights[3].position", pointLightPositions[3]);
    ls->setVec3("pointLights[3].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[3].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[3].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[3].constant", 1.0f);
    ls->setFloat("pointLights[3].linear", 0.014f);
    ls->setFloat("pointLights[3].quadratic", 0.0007f);
    // point light 5
    ls->setVec3("pointLights[4].position", pointLightPositions[4]);
    ls->setVec3("pointLights[4].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[4].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[4].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[4].constant", 1.0f);
    ls->setFloat("pointLights[4].linear", 0.014f);
    ls->setFloat("pointLights[4].quadratic", 0.0007f);
    // point light 6
    ls->setVec3("pointLights[5].position", pointLightPositions[5]);
    ls->setVec3("pointLights[5].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[5].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[5].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[5].constant", 1.0f);
    ls->setFloat("pointLights[5].linear", 0.014f);
    ls->setFloat("pointLights[5].quadratic", 0.0007f);
    // point light 7
    ls->setVec3("pointLights[6].position", pointLightPositions[6]);
    ls->setVec3("pointLights[6].ambient", pointStarLightColor * glm::vec3(0.8f));
    ls->setVec3("pointLights[6].diffuse", pointStarLightColor * glm::vec3(1.0f));
    ls->setVec3("pointLights[6].specular", 0.8f, 0.8f, 1.0f);
    ls->setFloat("pointLights[6].constant", 1.0f);
    ls->setFloat("pointLights[6].linear", 0.014f);
    ls->setFloat("pointLights[6].quadratic", 0.0007f);

    // spot lights
    int i = 0;
    for (glm::vec3 pos : spotLightPositions) {
        ls->setVec3("spotLights[" + std::to_string(i) + "].position", pos);
        ls->setVec3("spotLights[" + std::to_string(i) + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ls->setFloat("spotLights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(25.0f)));
        ls->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(40.0f)));
        ls->setVec3("spotLights[" + std::to_string(i) + "].ambient", 0.4f, 0.4f, 0.4f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].linear", 0.014f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.0007f);
        i++;
    }
    // spot lights do segundo hangar
    i = 9;
    for (glm::vec3 pos : spotLightPositions) {
        ls->setVec3("spotLights[" + std::to_string(i) + "].position", (pos + enemyHangarPos + glm::vec3(20.0f, 0.0f, 0.0f)));
        ls->setVec3("spotLights[" + std::to_string(i) + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ls->setFloat("spotLights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(25.0f)));
        ls->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(40.0f)));
        ls->setVec3("spotLights[" + std::to_string(i) + "].ambient", 0.4f, 0.4f, 0.4f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
        ls->setVec3("spotLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].linear", 0.014f);
        ls->setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.0007f);
        i++;
    }
}

/**
 * @brief Esta função renderiza uma string numa dada posição da tela, com uma cor e fonte desejada.
 */
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color, bool useFirstFont = true)
{
    textShader->use();
    glm::mat4 projection;
    projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader->setMat4("projection", projection);
    glUniform3f(glGetUniformLocation(textShader->ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0); 
    glBindVertexArray(VAOt);

    std::map<GLchar, Character>& fontCharacters = useFirstFont ? Characters : Characters2; // Modify this line to use the second font's Characters map

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = fontCharacters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBOt);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale; 
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderProjectiles(Shader& shader) {
    shader.use();

    for (const auto& proj : projectiles) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), proj.position);
        model = glm::scale(model, glm::vec3(2.0f, 7.0f, 2.0f)); // Escala aumentada para maior visibilidade
        shader.setMat4("model", model);

        if (proj.origin == 0) {
            shader.setVec3("boxColor", glm::vec3(0.0f, 1.0f, 0.0f)); // Verde para o jogador
        } else {
            shader.setVec3("boxColor", glm::vec3(1.0f, 0.0f, 0.0f)); // Vermelho para inimigos
        }

        // Configure as matrizes de visão e projeção
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f));

        glBindVertexArray(VAOProjectile);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numProjectileVertices); // 2 * (36 segmentos)
        glBindVertexArray(0);

        // std::cout << "Projectile rendered at position: " << proj.position.x << ", " << proj.position.y << ", " << proj.position.z << std::endl;

    }
    // std::cout << "Projectiles rendered. Count: " << projectiles.size() << std::endl;

}

/**
 * @brief Esta função renderiza o texto apresentado na introdução do jogo, 
 * indo alterando a posição de cada string dando um efeito de scroll
 */
void renderIntro(){
    // Clear the screen
    glClearColor(0.01f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const std::string introText[] = {
        "The   Rebel   Alliance,   battered   but   defiant   has,     ",
        "established   a   hidden   staging     ground    in   the     ",
        "remote   Outer   Rim.   But   the   relentless Imperial       ",
        "Fleet,   under  the  command  of  the  cunning  Grand         ",
        "Admiral   Thrawn,   has   discovered   their   location.      ",
        "Imperial    intelligence   has    uncovered   a   critical    ",
        "weakness   in  the   Rebel  defenses:  the  vulnerable        ",
        "hangar   bay   of   the   Cruiser   Liberator.   A    swift   ",
        "strike   against  this   crucial   vessel   could   cripple   ",
        "the   Rebel  fleet   before   it  can  launch  a  counter-    ",
        "offensive. Now,  as  the   Imperial  armada  descends         ",
        "from    the     stars,   the    fate    of   the     Rebellion",
        "hangs   in   the   balance . . .                              "
    };

    static float scrollOffset = 0.0f;
    scrollOffset += 5.0f; // Adjust the speed of scrolling here

    for (int i = 0; i < 13; ++i) {
        float textWidth = introText[i].length() * 20.0f;
        float yPos = SCR_HEIGHT / 2.0f - 200.0f - i * 80.0f + scrollOffset; 
        if (yPos > -50.0f && yPos < SCR_HEIGHT + 50.0f) {
            RenderText(introText[i], (SCR_WIDTH - textWidth) / 2.0f, yPos, 1.0f, textColor, true);
        }
    }

    if (scrollOffset > 160.0f + 13 * 80.0f) {
        scrollOffset = 0.0f; // Reset the scroll after the text has scrolled off the screen
        gameState = 0; // Change to game mode 0
    }
}

void renderBoundingBox(glm::vec3 position, float radius, float scaleFactor) {
    // std::cout << "Rendering bounding box at position: " 
    //           << position.x << ", " << position.y << ", " << position.z 
    //           << " with radius: " << radius << std::endl;

    glDisable(GL_DEPTH_TEST);  // Disable depth testing
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Wire frame mode

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(radius * scaleFactor));

    hitBoxShader->use();
    hitBoxShader->setMat4("model", model);
    hitBoxShader->setMat4("view", camera.GetViewMatrix());
    hitBoxShader->setMat4("projection", glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f));
    hitBoxShader->setVec3("boxColor", glm::vec3(1.0f, 0.0f, 0.0f));

    glBindVertexArray(boundingBoxVAO);
    glLineWidth(2.0f);  // Make lines thicker
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Restore fill mode
    glEnable(GL_DEPTH_TEST);  // Re-enable depth testing
}

void desenhaAlvo(){
    // Desabilitar o teste de profundidade para desenhar a mira na frente de tudo
    glDisable(GL_DEPTH_TEST);

    // Salvar a matriz de projeção e visualização atuais
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Definir a cor da mira (branco)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Definir a largura da linha
    glLineWidth(2.0f);

    // Desenhar a mira no centro da tela
    glBegin(GL_LINES);
    // Linha horizontal
    glVertex2f(0.49f, 0.5f);
    glVertex2f(0.51f, 0.5f);
    // Linha vertical
    glVertex2f(0.5f, 0.49f);
    glVertex2f(0.5f, 0.51f);
    glEnd();

    // Restaurar a matriz de projeção e visualização
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Habilitar o teste de profundidade novamente
    glEnable(GL_DEPTH_TEST);
}
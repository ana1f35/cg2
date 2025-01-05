#include "headers/texto.h"

/**
 * @brief Esta função inicializa strings para serem renderizadas
 * 
 * Caso o jogo esteja em estado inicial deverá ser renderizado o titulo do jogo e a instrução de inciar.
 */
void textoInicio(){
    const std::string titulo = "- War of StArs -";
    const std::string iniciar = "Press SPACE To Start";
    float textWidth2 = titulo.length() * 42.0f; 
    RenderText(titulo, (SCR_WIDTH - textWidth2) / 2.0f, SCR_HEIGHT / 2.0f + 50.0f, 1.0f, textColor, false);
    float textWidth = iniciar.length() * 25.0f; 
    RenderText(iniciar, (SCR_WIDTH - textWidth) / 2.0f, SCR_HEIGHT / 2.0f - 50.0f, 1.0f, textColor, true);

    return;
}

/**
 * @brief Esta função inicializa strings para serem renderizadas
 * 
 * Durante o jogo será mostrada a pontuação e os pontos de vida atuais.
 */
void textoDurante(){
    std::string vida = "Health: " + std::to_string(fighter_player.hp);
    std::string pontuacaoStr = "Score: " + std::to_string(pontuacao);
    RenderText(vida, SCR_WIDTH - 300.0f, SCR_HEIGHT - 100.0f, 1.0f, textColor, true);
    RenderText(pontuacaoStr, SCR_WIDTH - 300.0f, SCR_HEIGHT - 180.0f, 1.0f, textColor, true);

    return;
}

/**
 * @brief Esta função inicializa strings para serem renderizadas
 * 
 * Permite indicar ao jogador os controlos do jogo.
 */
void textoPausa(){
    const std::string controlsTitle = "Game Controls";
    const std::string controlsText1 = "W - Increase Speed";
    const std::string controlsText2 = "S - Decrease Speed";
    const std::string controlsText3 = "Q - Attack";
    const std::string controlsText4 = "V - Toggle Camera View";
    const std::string controlsText6 = "ESC - Exit Game";
    const std::string controlsText7 = "C - Close Controls Menu";

    float titleWidth = controlsTitle.length() * 48.0f;
    RenderText(controlsTitle, (SCR_WIDTH - titleWidth) / 2.0f, SCR_HEIGHT / 2.0f + 380.0f, 1.0f, textColor, false);
    float textWidth1 = controlsText1.length() * 25.0f;
    RenderText(controlsText1, (SCR_WIDTH - textWidth1) / 2.0f, SCR_HEIGHT / 2.0f + 160.0f, 1.0f, textColor, true);
    float textWidth2 = controlsText2.length() * 25.0f;
    RenderText(controlsText2, (SCR_WIDTH - textWidth2) / 2.0f, SCR_HEIGHT / 2.0f + 80.0f, 1.0f, textColor, true);
    float textWidth3 = controlsText3.length() * 25.0f;
    RenderText(controlsText3, (SCR_WIDTH - textWidth3) / 2.0f, SCR_HEIGHT / 2.0f, 1.0f, textColor, true);
    float textWidth4 = controlsText4.length() * 25.0f;
    RenderText(controlsText4, (SCR_WIDTH - textWidth4) / 2.0f, SCR_HEIGHT / 2.0f - 80.0f, 1.0f, textColor, true);
    float textWidth6 = controlsText6.length() * 25.0f;
    RenderText(controlsText6, (SCR_WIDTH - textWidth6) / 2.0f, SCR_HEIGHT / 2.0f - 250.0f, 1.0f,  textColor, true);
    float textWidth7 = controlsText7.length() * 25.0f;
    RenderText(controlsText7, (SCR_WIDTH - textWidth7) / 2.0f, SCR_HEIGHT / 2.0f - 350.0f, 1.0f, textColor, true);

    return;  
}

/**
 * @brief Esta função inicializa strings para serem renderizadas
 * 
 * No final do jogo deverá ser apresentada a pontuação final e os pontos de vida restantes.
 */
void textoFinal(){
    std::string gameOver = "Game over";
    std::string finalScore = "Final Score: " + std::to_string(pontuacao);
    std::string finalHealth = "HP Left: " + std::to_string(fighter_player.hp);
    std::string restart = "Press SPACE To Restart";
    float gameOverWidth = gameOver.length() * 52.0f;
    RenderText(gameOver, (SCR_WIDTH - gameOverWidth) / 2.0f, SCR_HEIGHT / 2.0f + 100.0f, 1.0f, textColor, false);
    float scoreWidth = finalScore.length() * 25.0f;
    RenderText(finalScore, (SCR_WIDTH - scoreWidth) / 2.0f, SCR_HEIGHT / 2.0f, 1.0f, textColor, true);
    float healthWidth = finalHealth.length() * 25.0f;
    RenderText(finalHealth, (SCR_WIDTH - healthWidth) / 2.0f, SCR_HEIGHT / 2.0f - 100.0f, 1.0f, textColor, true);
    float restartWidth = restart.length() * 25.0f;
    RenderText(restart, (SCR_WIDTH - restartWidth) / 2.0f, SCR_HEIGHT / 2.0f - 200.0f, 1.0f, textColor, true);
        
    return;
}


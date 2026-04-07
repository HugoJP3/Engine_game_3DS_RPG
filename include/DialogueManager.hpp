#pragma once
#include <citro2d.h>
#include <vector>
#include <string>
#include <math.h>
#include <iostream>
#include "utils/Config.hpp"
#include "entities/Entity.hpp"

struct DialogueChoice {
    std::string text; // Elección
    std::vector<std::pair<std::string, bool>> flagsOnSelect; // Flags asociadas a elecciones
};

struct DialogueBranch {
    std::string conditionFlag; // Si esta flag es true...
    bool expectedValue;        // ...y coincide con este valor...
    std::vector<std::string> lines; // ...dice esto.
    std::vector<std::pair<std::string, bool>> setFlagsOnEnd; // ... flags activas después del diálogo

    std::vector<DialogueChoice> choices;
    bool hasChoices = false;
};

class DialogueManager {
    private:
        FlagManager* flagManager;

        // Datos diálogo
        DialogueBranch currentBranch;
        std::string character_name = "";
        std::vector<DialogueBranch> allBranches;
        
        size_t currentLineIdx = 0;
        size_t charIdx = 0;

        bool inChoiceMode = false;
        size_t selectedChoice = 0;
        
        C2D_SpriteSheet ui_dialogue;        
        C2D_TextBuf text_Buffer, nombre_Buffer;

        float animFrame = 0.0f; // vel caracteres
        float velWriting = 0.05f; // vel caracteres
        float animTime = 0.0f; // oscilaciones elección

        bool active = false;

        DialogueBranch getNextBranch();

    public:
        DialogueManager(FlagManager* flagManager);
        ~DialogueManager();

        void startDialogue(const std::vector<DialogueBranch>& branches, std::string name);
        void update(float dt, u32 kDown);

        void draw();
        
        void call_expression(float draw_x, float draw_y, Expression expresion);
        void call_expression(Entity* ent, float camX, float camY);
        
        bool isActive() const { return active; }
};
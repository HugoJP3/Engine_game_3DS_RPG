#include <string>
#include <states/FlagManager.hpp>
#include "entities/Entity.hpp"
#include "DialogueManager.hpp"

class NPC : public Entity {
    private:
        std::vector<DialogueBranch> branches;
        std::string npcID;
        int baseIndex; // base de animación
        std::string nombre;

        int animate = 0; // se anima o es estático
        int spriteIdx; // índice en sprite sheet
        float velAnimation = 0.15f; // velocidad animación

        float animTimer = 0.0f; // 0-0.15f

        Expression currentMood = CHAT;

        int voiceTone = -1;

        std::vector<std::pair<std::string,bool>> showFlags;

    public:
        NPC(float x, float y, int width, int height, int colW, int colH, std::string name, FlagManager* flagManager, int animate, int spriteIdx)
            : Entity(x, y, 0.35,
                    width, height,
                    colW, colH,
                    flagManager),
            baseIndex(0), nombre(name), animate(animate), spriteIdx(spriteIdx) {}

        void init() {}

        void addBranch(DialogueBranch db) {
            branches.push_back(db);
        }

        const DialogueBranch& getCurrentDialogo() {
            // Devuelve el que coincida con la flag actual
            for (int i = branches.size() - 1; i >= 0; i--) {
                if (branches[i].conditionFlag.empty() || branches[i].conditionFlag == "none")
                    continue;

                if (flagManager->getFlag(branches[i].conditionFlag) == branches[i].expectedValue) {
                    return branches[i];
                }
            }

            // Si no tengo flags activas, buscamos la rama por defecto (none).
            for (const auto& b : branches) {
                if (b.conditionFlag == "none" || b.conditionFlag.empty()) {
                    return b;
                }
            }
            
            return branches[0];
        }
        
        std::string getNombre() const { return nombre; }

        void setVoiceTone(int t) {
            if (t < 0) voiceTone = -1;
            else if (t > DialogueManager::VOICE_TONE_INDEX_MAX) voiceTone = DialogueManager::VOICE_TONE_INDEX_MAX;
            else voiceTone = t;
        }

       void updateY(float yPlayerCenter) {
            float npcFeet = y + collision.offsetY + collision.height;
            if (yPlayerCenter < npcFeet)
                z = 0.45f;   // NPC delante
            else
                z = 0.35f;   // NPC detrás
        }


        void update(float dt) override {
            // Animación
            if (animate > 1) {
                animTimer += dt;
                if (animTimer >= velAnimation) {
                    animTimer = 0.0f;
                    baseIndex = (baseIndex + 1) % animate;
                }
            }
        }

        void draw(float camX, float camY) override {
            if (!spriteSheet) return;

            C2D_Image img = C2D_SpriteSheetGetImage(spriteSheet, baseIndex + spriteIdx);
            
            C2D_DrawImageAt(img, getRenderX(camX), getRenderY(camY), z, NULL, Config::globalScale, Config::globalScale);

            if (Config::showColissions) {
                drawCollision(camX, camY);
            }
        }

        Expression getInteractableExpression() const override { return currentMood; }
        void setMood(Expression m) { currentMood = m; }

        void onInteract(InteractionContext& ctx) {
            if (ctx.dialogueManager) {
                ctx.dialogueManager->startDialogue(
                    this->branches,
                    this->getName(),
                    voiceTone
                );
            }
        }

        bool isVisible() {
            this->setSolid(true);
            if (showFlags.empty()) return true;

            for (auto& sf : showFlags) {
                const std::string& flag = sf.first;
                bool expected = sf.second;

                if (flagManager->getFlag(flag) != expected)    
                {            
                    this->setSolid(false);
                    return false;
                }
            }
            return true;
        }


        std::string getName() { return nombre; }
        void setName(const std::string& n) { nombre = n; }

        void setPosition(float x, float y) { setX(x); setY(y); }
        void setVel(float vel) { velAnimation = vel; }

        void addShowFlag(const std::string& flag, bool expectedValue) {
            showFlags.push_back({flag, expectedValue});
        }
};
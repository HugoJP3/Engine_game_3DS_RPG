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

        float animTimer = 0.0f; // 0-0.15f

    public:
        NPC(float x, float y, int width, int height, std::string name, FlagManager* flagManager, int animate, int spriteIdx)
            : Entity(x, y, 0.35,
                    width, height,
                    width, height,
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

        void updateY(float yPlayer) {
            // Variar altura con la del personaje
            if (yPlayer < y) z = 0.45f;
            else z = 0.35f;
        }

        void update(float dt) override {
            // Animación
            if (animate) {
                animTimer += dt;
                if (animTimer >= 0.15f) {
                    animTimer = 0.0f;
                    baseIndex = (baseIndex + 1) % animate;
                    if (baseIndex == 0) baseIndex = 1; 
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

        void onInteract(InteractionContext& ctx) {
            if (ctx.dialogueManager) {
                ctx.dialogueManager->startDialogue(
                    this->getCurrentDialogo(),
                    this->getName()
                );
            }
        }

        std::string getName() { return nombre; }
        void setName(const std::string& n) { nombre = n; }

        void setPosition(float x, float y) { setX(x); setY(y); }
};
// Estado actual y cambio de estados
#pragma once
#include <algorithm>
#include "states/State.hpp"
#include "states/FlagManager.hpp"
#include "states/Inventory.hpp"

class SceneManager {
    private:
        static constexpr float FADE_SPEED = 200.0f;
        // Evita saltos enormes de alpha si un frame tarda mucho (fade-in “invisible”).
        static constexpr float FADE_DT_CAP = 1.0f / 30.0f;
        enum FadePhase { FADE_IDLE, FADE_OUT, FADE_IN };

        C3D_RenderTarget* topTarget;
        State* currentState = nullptr;
        State* nextState = nullptr;

        FlagManager flagManager;

        Inventory* inventario = nullptr;
        C3D_RenderTarget* bottomTarget;
        
        float fadeAlpha = 0.0f; // 0 = transparente, 255 = negro
        FadePhase fadePhase = FADE_IDLE;
    
    public:
        SceneManager(C3D_RenderTarget* top, C3D_RenderTarget* bottom) {
            currentState = nullptr;
            topTarget = top;
            bottomTarget = bottom;
            inventario = new Inventory(bottomTarget);
        }

        void changeState(State* newState) {
            if (fadePhase != FADE_IDLE) return;
            nextState = newState;
            fadePhase = FADE_OUT;
        }

        void update(float dt, u32 kDown) {
            const float fadeDt = std::min(dt, FADE_DT_CAP);

            if (fadePhase == FADE_OUT) {
                fadeAlpha += FADE_SPEED * fadeDt;
                if (fadeAlpha >= 255.0f) {
                    fadeAlpha = 255.0f;

                    if (currentState) {
                        delete currentState; 
                        currentState = nullptr;
                    }

                    currentState = nextState;
                    nextState = nullptr;

                    if (currentState) {
                        currentState->setManager(this);
                        currentState->init();
                    }
                    fadePhase = FADE_IN;
                }
                return;
            }

            if (fadePhase == FADE_IN) {
                fadeAlpha -= FADE_SPEED * fadeDt;
                if (fadeAlpha <= 0.0f) {
                    fadeAlpha = 0.0f;
                    fadePhase = FADE_IDLE;
                }
            }

            if (currentState) currentState->update(dt, kDown);
        }

        void draw() {
            if (currentState) currentState->draw();

            // Fade en la misma escena 2D del estado (WorldState deja top activo).
            // Un segundo C2D_SceneBegin(top) aquí puede vaciar/reiniciar la escena y romper el fade-in.
            if (fadeAlpha > 0.0f) {
                const u8 a = (u8)std::min(255.0f, fadeAlpha);
                C2D_DrawRectSolid(0, 0, 1.0f, 400, 240, C2D_Color32(0, 0, 0, a));
            }

            inventario->draw();
        }

        ~SceneManager() {
            if (currentState) delete currentState;
            if (inventario) delete inventario;
        }

        Inventory* getInventory() { return inventario; }
};
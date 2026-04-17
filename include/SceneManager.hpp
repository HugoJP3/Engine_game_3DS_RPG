// Estado actual y cambio de estados
#pragma once
#include "states/State.hpp"
#include "states/FlagManager.hpp"
#include "states/Inventory.hpp"

class SceneManager {
    private:
        C3D_RenderTarget* topTarget;
        State* currentState = nullptr;
        State* nextState = nullptr;

        FlagManager flagManager;

        Inventory* inventario = nullptr;
        C3D_RenderTarget* bottomTarget;
        
        float fadeAlpha = 0.0f; // 0 = transparente, 255 = negro
        bool isFading = false;
    
    public:
        SceneManager(C3D_RenderTarget* top, C3D_RenderTarget* bottom) {
            currentState = nullptr;
            topTarget = top;
            bottomTarget = bottom;
            inventario = new Inventory(bottomTarget);
        }

        void changeState(State* newState) {
            if (isFading) return;
            nextState = newState;
            isFading = true;
        }

        void update(float dt, u32 kDown) {
            if (isFading) {
                fadeAlpha += 500.0f * dt;
                if (fadeAlpha >= 255.0f) {
                    fadeAlpha = 255.0f;

                    if (currentState) {
                        delete currentState; 
                        currentState = nullptr;
                    }
                    
                    C3D_FrameEnd(0);

                    currentState = nextState;
                    nextState = nullptr;

                    if (currentState) {
                        currentState->setManager(this);
                        currentState->init();
                    }
                    isFading = false;
                }
            } else if (fadeAlpha > 0.0f) {
                fadeAlpha -= 500.0f * dt;
                if(fadeAlpha < 0.0f) fadeAlpha = 0.0f;
            }

            if (currentState) currentState->update(dt, kDown);
        }

        void draw() {
            if (currentState) currentState->draw();

            inventario->draw();

            if (fadeAlpha > 0.0f) {
                C2D_SceneBegin(topTarget);
                C2D_DrawRectSolid(0, 0, 0.9f, 400, 240, C2D_Color32(0, 0, 0, (u8)fadeAlpha));
            }
        }

        ~SceneManager() {
            if (currentState) delete currentState;
            if (inventario) delete inventario;
        }

        Inventory* getInventory() { return inventario; }
};
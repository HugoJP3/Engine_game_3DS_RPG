// Interfaz de estados
#pragma once
#include "ResourceManager.hpp"

class SceneManager;

class State {
    protected:
        SceneManager* manager = nullptr;

    public:
        virtual ~State() {}
        virtual void init() = 0; // Carga recursos
        virtual void update(float dt, u32 kDown) = 0; // Lógica
        virtual void draw() = 0; // Renderizado

        void setManager(SceneManager* m) {
            manager = m;
        }
};
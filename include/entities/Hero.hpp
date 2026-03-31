// Características del Hero
#pragma once
#include "entities/Entity.hpp"
#include "hero_data.hpp"

class Hero : public Entity {
    private:
        static constexpr float velocidad = 150.0f; // Velocidad movimiento
        float dx = 0, dy = 0; // X, Y anteriores a movimiento

        float animTimer = 0.0f; // 0-0.15f
        int animFrame = 0; // 0, 1, 2
        bool moviendose = false;
        HeroDirection currentDir = DIR_FRONT;

        // Colisiones dinámicas
        float colWidthFront, colHeightFront; // De frente, el tamaño cambia
        float colWidthLateral, colHeightLateral; // De lado, el tamaño cambia
        float offsetXFront, offsetYFront;
        float offsetXLateral, offsetYLateral;

    public:
        Hero(float x, float y, float z, FlagManager* flagManager);
        ~Hero();
        
        void init() override;
        void update(float dt) override;
        void draw(float camX, float camY) override;

        void onInteract(InteractionContext& ctx) override {}
        
        void moveX(float dt);
        void moveY(float dt);
};
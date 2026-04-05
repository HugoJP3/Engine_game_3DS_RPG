// Características del Hero
#include "entities/Hero.hpp"
#include <3ds.h>

Hero::Hero(float x, float y, float z, FlagManager* flagManager) 
    : Entity(x, y, z, 25.0f, 15.0f,
        0, 0, flagManager) 
    {
        setCollision(
            23.0f, 8.0f,
            (25.0f - 23.0f) / 2.0f,
            15.0f - 8.0f + 2.0f
        );
    }

void Hero::init() {
    currentDir = DIR_SIDE_RIGHT;
}

void Hero::update(float dt) {
    circlePosition pos;
    hidCircleRead(&pos);

    int deadzone = 20;
    moviendose = false;
    dx = 0; dy = 0;

    // Lógica de movimiento
    if (abs(pos.dx) > deadzone || abs(pos.dy) > deadzone) {
        dx = pos.dx / 156.0f;
        dy = pos.dy / 156.0f;
        moviendose = true;

        // Determinar dirección para animación y colisiones dinámicas
        //if (abs(dx) > abs(dy)) {
        //    currentDir = (dx > 0) ? DIR_SIDE_RIGHT : DIR_SIDE_LEFT;
        //}
        //else {
        //    currentDir = (dy > 0) ? DIR_BACK : DIR_FRONT;
        //}

        currentDir = (dx > 0) ? DIR_SIDE_RIGHT : DIR_SIDE_LEFT;

    }

    // Animación
    if (moviendose) {
        animTimer += dt;
        if (animTimer >= 0.15f) {
            animTimer = 0.0f;
            animFrame = (animFrame + 1) % 3;
            if (animFrame == 0) animFrame = 1; 
        }
    } else {
        animFrame = 0;
    }
}

void Hero::moveX(float dt) {
    x += dx * velocidad * dt;
}

void Hero::moveY(float dt) {
    y -= dy * velocidad * dt;
}

void Hero::draw(float camX, float camY) {
    int baseIndex = 0;

    if (currentDir == DIR_SIDE_LEFT) baseIndex = SPR_HERO_LEFT_IDLE;
    else if (currentDir == DIR_SIDE_RIGHT) baseIndex = SPR_HERO_RIGHT_IDLE;
    else if (currentDir == DIR_BACK) baseIndex = SPR_HERO_BACK_IDLE;
    else baseIndex = SPR_HERO_FRONT_IDLE;

    C2D_Image img = C2D_SpriteSheetGetImage(spriteSheet, baseIndex + animFrame);
    
    C2D_DrawImageAt(img, getRenderX(camX), getRenderY(camY), z, NULL, Config::globalScale, Config::globalScale);

    if (Config::showColissions) {
        // Solo para debug
        C2D_DrawRectSolid((x + collision.offsetX - camX) * Config::globalScale, (y + collision.offsetY - camY) * Config::globalScale, z + 0.1f,
            collision.width * Config::globalScale, collision.height * Config::globalScale, C2D_Color32(255, 0, 0, 150));
    }
}

Hero::~Hero() {}
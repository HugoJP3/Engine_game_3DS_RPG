// Características del Hero
#include "entities/Hero.hpp"
#include <3ds.h>

Hero::Hero(float x, float y, float z, FlagManager* flagManager) 
    : Entity(x, y, z,
        48.0f * Config::globalScale, // ancho
        48.0f * Config::globalScale, // alto
        0, 0, flagManager) 
    {
        // Cajas de colisión
        colWidthFront = 16.0f * Config::globalScale;
        colHeightFront = 24.0f * Config::globalScale;
        colWidthLateral = colWidthFront;
        colHeightLateral = colHeightFront;
        
        // Offsets centrados y abajo (en los pies)
        offsetXFront = (width - colWidthFront) / 2.0f;
        offsetYFront = height - colHeightFront;
        
        offsetXLateral = offsetXFront;
        offsetYLateral = offsetYFront;

        // Valores iniciales
        colWidth = colWidthFront;
        colHeight = colHeightFront;
        offsetX = offsetXFront;
        offsetY = offsetYFront;

    }

void Hero::init() {}

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
        if (abs(dx) > abs(dy)) {
            currentDir = (dx > 0) ? DIR_SIDE_RIGHT : DIR_SIDE_LEFT;
            
            colHeight = colHeightLateral;
            colWidth = colWidthLateral;
            offsetX = offsetXLateral;
            offsetY = offsetYLateral;
        } else {
            currentDir = (dy > 0) ? DIR_BACK : DIR_FRONT;
            colHeight = colHeightFront;
            colWidth = colWidthFront;
            offsetX = offsetXFront;
            offsetY = offsetYFront;
        }
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
    
    C2D_DrawImageAt(img, x - camX, y - camY, z, NULL, Config::globalScale, Config::globalScale);

    if (Config::showColissions) {
        // Solo para testeo (borrar luego)
        C2D_DrawRectSolid(x + offsetX - camX, y + offsetY - camY, z + 0.1f, colWidth, colHeight, C2D_Color32(255, 0, 0, 150));
    }
}

Hero::~Hero() {}
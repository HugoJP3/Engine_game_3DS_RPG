#pragma once
#include <stdio.h>
#include <citro2d.h>
#include "utils/Config.hpp"

class DialogueManager;
class Inventory;

typedef enum {
    ALERTED = 3,
    CHAT,
    CONFUSED
} Expression;

// Cuando se dé una interacción básica
struct InteractionContext {
    DialogueManager* dialogueManager;
    Inventory* inventory;
};

struct Collision {
    float width, height;
    float offsetX, offsetY;
};

class Entity {
    protected:
        // General
        float x, y, z;
        float width, height; // Tamaño lógico, sin escala

        // Lógica colisiones, sin escala
        Collision collision;

        bool solid = false;

        C2D_SpriteSheet spriteSheet;

        FlagManager* flagManager;

        bool pendingRemoval = false; // Variable que le indica al manager que debe eliminar la instancia

        void drawCollision(float camX, float camY) {
            u32 colorRect = C2D_Color32(0, 255, 0, 100);
            float debugZ = 0.8f;
            float thickness = 5.0f;
            float drawX = (x + collision.offsetX - camX) * Config::globalScale;
            float drawY = (y + collision.offsetY - camY) * Config::globalScale;
            float scaledSizeX = collision.width * Config::globalScale;
            float scaledSizeY = collision.height * Config::globalScale;

            C2D_DrawLine(drawX, drawY, colorRect, drawX + scaledSizeX, drawY, colorRect, thickness, debugZ); // Top
            C2D_DrawLine(drawX, drawY + scaledSizeY, colorRect, drawX + scaledSizeX, drawY + scaledSizeY, colorRect, thickness, debugZ); // Bottom
            C2D_DrawLine(drawX, drawY, colorRect, drawX, drawY + scaledSizeY, colorRect, thickness, debugZ); // Left
            C2D_DrawLine(drawX + scaledSizeX, drawY, colorRect, drawX + scaledSizeX, drawY + scaledSizeY, colorRect, thickness, debugZ); // Right
        } 
        
    public:
        Entity(float x, float y, float z,
            float w, float h,
            float colW, float colH,
            FlagManager* flagManager)
            : x(x), y(y), z(z),
            width(w), height(h),
            spriteSheet(nullptr), flagManager(flagManager),
            pendingRemoval(false) {
                collision = {colW, colH, 0, 0};
                if (height != colH) collision.offsetY = (height - colH);
            }
        
        virtual ~Entity() {}

        virtual void init() = 0;        
        virtual void update(float dt) = 0;
        virtual void draw(float camX, float camY) = 0;

        // --- INTERACCIÓN/FEEDBACK CON HERO ---
        virtual void onInteract(InteractionContext& ctx) = 0;
        virtual Expression getInteractableExpression() const = 0;

        // --- INICIALIZACIÓN ---
        void setSpriteSheet(C2D_SpriteSheet sheet) { spriteSheet = sheet; }

        void setSolid(bool s) { solid = s; }
        bool isSolid() const { return solid; }

        // --- POSICIÓN ---
        float getX() const { return x; }
        float getY() const { return y; }

        void setX(float newX) { x = newX; }
        void setY(float newY) { y = newY; }

        // --- TAMAÑO LÓGICO ---
        float getWidth() const { return width; }
        float getHeight() const { return height; }

        // --- HITBOX ---
        float getColWidth() const {return collision.width; }
        float getColHeight() const {return collision.height; }

        float getOffsetX() const { return collision.offsetX; }
        float getOffsetY() const { return collision.offsetY; }

        void setCollision(float w, float h, float offX, float offY) { collision = {w, h, offX, offY}; }
        void setCollision(Collision c) { collision = c; }

        // --- CENTRO ---
        float getCenterX() const { return x + width / 2.0f; }
        float getCenterY() const { return y + height / 2.0f; }

        // --- RENDER SPACE ---
        float getRenderWidth() const { return width * Config::globalScale; }
        float getRenderHeight() const { return height * Config::globalScale; }

        float getRenderX(float camX) const { return (x - camX) * Config::globalScale; }
        float getRenderY(float camY) const { return (y - camY) * Config::globalScale; }

        // --- MÉTODO COLISIÓN ---
        bool checkCollision(Entity& other)
        {         
            float offset1X = collision.offsetX;
            float offset1Y = collision.offsetY;
            
            float left1 = x + offset1X;
            float right1 = x + offset1X + collision.width;
            float top1 = y + offset1Y;
            float bottom1 = y + offset1Y + collision.height;

            float offset2X = other.getOffsetX();
            float offset2Y = other.getOffsetY();

            float left2 = other.getX() + offset2X;
            float right2 = other.getX() + offset2X + other.getColWidth();
            float top2 = other.getY() + offset2Y;
            float bottom2 = other.getY() + offset2Y + other.getColHeight();

            return (left1 < right2 && right1 > left2 && top1 < bottom2 && bottom1 > top2);
        }

        // --- ESTADO A ELIMINAR ---
        bool isPendingRemoval() const { return pendingRemoval; }
        void setPendingRemoval(bool pr) { pendingRemoval = pr; }
};
#pragma once
#include <stdio.h>
#include <citro2d.h>
#include "utils/Config.hpp"

class DialogueManager;
class Inventory;

// Cuando se dé una interacción básica
struct InteractionContext {
    DialogueManager* dialogueManager;
    Inventory* inventory;
};

class Entity {
    protected:
        // General
        float x, y, z;
        float width, height; // Tamaño lógico, sin escala

        // Lógica colisiones, sin escala
        float colWidth, colHeight;
        float offsetX = 0.0f, offsetY = 0.0f;

        bool solid = false;

        C2D_SpriteSheet spriteSheet;

        FlagManager* flagManager;

        bool pendingRemoval = false; // Variable que le indica al manager que debe eliminar la instancia

        void drawCollision(float camX, float camY) {
            u32 colorRect = C2D_Color32(0, 255, 0, 100);
            float debugZ = 0.8f;
            float thickness = 5.0f;
            float drawX = (x + offsetX - camX) * Config::globalScale;
            float drawY = (y + offsetY - camY) * Config::globalScale;
            float scaledSizeX = colWidth;
            float scaledSizeY = colHeight;

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
            colWidth(colW), colHeight(colH),
            offsetX(0), offsetY(0),
            spriteSheet(nullptr), flagManager(flagManager),
            pendingRemoval(false) {}
        
        virtual ~Entity() {}

        virtual void init() = 0;        
        virtual void update(float dt) = 0;
        virtual void draw(float camX, float camY) = 0;

        virtual void onInteract(InteractionContext& ctx) = 0;

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
        float getColWidth() const {return colWidth; }
        float getColHeight() const {return colHeight; }

        float getOffsetX() const { return offsetX; }
        float getOffsetY() const { return offsetY; }

        void setCollision(float w, float h, float offX, float offY) {
            colWidth = w;
            colHeight = h;
            offsetX = offX;
            offsetY = offY;
        }

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
            float offset1X = offsetX;
            float offset1Y = offsetY;
            
            float left1 = x + offset1X;
            float right1 = x + offset1X + colWidth;
            float top1 = y + offset1Y;
            float bottom1 = y + offset1Y + colHeight;

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
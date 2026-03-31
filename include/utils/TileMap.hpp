#pragma once
#include <citro2d.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "utils/Config.hpp"
#include "utils/collisionTypes.hpp"

class TileMap {
private:
    std::vector<std::vector<int>> data; // Matriz tiles

    C2D_SpriteSheet sheet;
    float layerZ = 0.1f;

    CollisionType myCollisionType = NO_COLLISION; // Matriz es de colisiones

public:
    TileMap(C2D_SpriteSheet s, float z, CollisionType collision = NO_COLLISION)
        : sheet(s), layerZ(z), myCollisionType(collision) {}

    void loadFromCSV(std::string path) {
        std::ifstream file(path);
        std::string line;
        data.clear();

        while (std::getline(file, line)) {
            std::vector<int> row;
            std::stringstream ss(line);
            std::string value;

            while (std::getline(ss, value, ',')) {
                row.push_back(std::stoi(value));
            }
            data.push_back(row);
        }
    }

    void draw(float camX, float camY) {
        float scaledSize = 16.0f * Config::globalScale;
        float margin = scaledSize;

        // --- DIBUJADO COLISIONES y TP ---
        if (Config::showColissions && (myCollisionType != NO_COLLISION)) {
            for (size_t y = 0; y < data.size(); y++) {
                for (size_t x = 0; x < data[y].size(); x++) {
                    if (data[y][x] != -1) {
                        float posX = x * scaledSize;
                        float posY = y * scaledSize;

                        if (posX >= camX - margin && posX <= camX + 400 + margin &&
                            posY >= camY - margin && posY <= camY + 240 + margin) {
                            
                            float drawX = floorf(posX - camX);
                            float drawY = floorf(posY - camY);

                            u32 colorRect = C2D_Color32(255, 0, 0, 100);
                            if ((myCollisionType == TP_NEXT) || (myCollisionType == TP_PREV)) 
                                {colorRect = C2D_Color32(0, 0, 255, 100);}
                                
                                float debugZ = 0.8f;
                                float thickness = 5.0f;
                                C2D_DrawLine(drawX, drawY, colorRect, drawX + scaledSize, drawY, colorRect, thickness, debugZ); // Top
                                C2D_DrawLine(drawX, drawY + scaledSize, colorRect, drawX + scaledSize, drawY + scaledSize, colorRect, thickness, debugZ); // Bottom
                                C2D_DrawLine(drawX, drawY, colorRect, drawX, drawY + scaledSize, colorRect, thickness, debugZ); // Left
                                C2D_DrawLine(drawX + scaledSize, drawY, colorRect, drawX + scaledSize, drawY + scaledSize, colorRect, thickness, debugZ); // Right
                        }
                    }
                }
            }
        }

        // --- DIBUJADO TILES ---
        if (myCollisionType == NO_COLLISION) {
            for (size_t y = 0; y < data.size(); y++) {
                for (size_t x = 0; x < data[y].size(); x++) {
                    int tileID = data[y][x];
                    
                    if (tileID != -1) {
                        float posX = x * scaledSize;
                        float posY = y * scaledSize;

                        // Culling
                        if (posX >= camX - margin && posX <= camX + 400 + margin &&
                            posY >= camY - margin && posY <= camY + 240 + margin) {
                            
                            float drawX = floorf(posX - camX);
                            float drawY = floorf(posY - camY);

                            C2D_Image img = C2D_SpriteSheetGetImage(sheet, tileID);
                            C2D_DrawImageAt(img, drawX, drawY, layerZ, NULL, 
                                Config::globalScale, Config::globalScale);
                        }
                    }
                }
            }
        }
    }

    bool isSolidAt(float worldX, float worldY) {
        if(myCollisionType == NO_COLLISION) return false;

        float scaledTileSize = 16.0f * Config::globalScale;

        int tx = std::floor(worldX / scaledTileSize);
        int ty = std::floor(worldY / scaledTileSize);

        if (ty >= 0 && (size_t)ty < data.size() &&
            tx >= 0 && (size_t)tx < data[ty].size()) {
                return data[ty][tx] != -1;
        }
        return false;
    }

    void setSolid(CollisionType newState) { myCollisionType = newState; }

    float getZ() { return layerZ; }

    void changeSheet(C2D_SpriteSheet s) { sheet = s; }
};
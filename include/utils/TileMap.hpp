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

    CollisionType myCollisionType = NO_COLLISION;

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

    void draw(float camX, float camY, float yPlayer) {
        float margin_world = Config::TILE_SIZE;
        const float screenW_world = 400.0f / Config::globalScale;
        const float screenH_world = 240.0f / Config::globalScale;

        // --- DIBUJADO COLISIONES y TP ---
        if (Config::showColissions && (myCollisionType != NO_COLLISION)) {
            for (size_t r = 0; r < data.size(); r++) {
                for (size_t c = 0; c < data[r].size(); c++) {
                    if (data[r][c] < 0) continue;
                    
                    float posX = c * Config::TILE_SIZE;
                    float posY = r * Config::TILE_SIZE;

                    if (posX >= camX - margin_world && posX <= camX + screenW_world + margin_world &&
                      posY >= camY - margin_world && posY <= camY + screenH_world + margin_world) {
                      
                        float drawX = std::floorf(posX - camX) * Config::globalScale;
                        float drawY = std::floorf(posY - camY) * Config::globalScale;

                        u32 colorRect = C2D_Color32(255, 0, 0, 100);
                        if (myCollisionType == TP) 
                            colorRect = C2D_Color32(0, 0, 255, 100);
                            
                        float debugZ = 0.8f;
                        float thickness = 5.0f;
                        float scaledSize = Config::TILE_SIZE * Config::globalScale;

                        C2D_DrawLine(drawX, drawY, colorRect, drawX + scaledSize, drawY, colorRect, thickness, debugZ);
                        C2D_DrawLine(drawX, drawY + scaledSize, colorRect, drawX + scaledSize, drawY + scaledSize, colorRect, thickness, debugZ);
                        C2D_DrawLine(drawX, drawY, colorRect, drawX, drawY + scaledSize, colorRect, thickness, debugZ);
                        C2D_DrawLine(drawX + scaledSize, drawY, colorRect, drawX + scaledSize, drawY + scaledSize, colorRect, thickness, debugZ);
                    }
                }
            }
        }

        // --- DIBUJADO TILES ---
        if (myCollisionType == NO_COLLISION) {
            for (size_t r = 0; r < data.size(); r++) {
                for (size_t c = 0; c < data[r].size(); c++) {
                    int tileID = data[r][c];
                    if (tileID < 0) continue;
                    
                    float posX = c * Config::TILE_SIZE;
                    float posY = r * Config::TILE_SIZE;

                    // Culling
                    if (posX >= camX - margin_world && posX <= camX + screenW_world + margin_world &&
                        posY >= camY - margin_world && posY <= camY + screenH_world + margin_world) {
                    
                        float drawX = std::floorf(posX - camX) * Config::globalScale;
                        float drawY = std::floorf(posY - camY) * Config::globalScale;

                        C2D_Image img = C2D_SpriteSheetGetImage(sheet, tileID);
                        C2D_DrawImageAt(img, drawX, drawY, layerZ, NULL, 
                            Config::globalScale, Config::globalScale);
                    }
                }
            }
        }
    }

    bool isSolidAt(float worldX, float worldY) {
        if(myCollisionType == NO_COLLISION) return false;

        int tx = std::floor(worldX / Config::TILE_SIZE);
        int ty = std::floor(worldY / Config::TILE_SIZE);

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
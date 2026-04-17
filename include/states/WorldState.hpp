// Estado mundo jugable
#pragma once
#include <vector>
#include <map>
#include <string>
#include <citro2d.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include "State.hpp"
#include "entities/Hero.hpp"
#include "entities/Object.hpp"
#include "entities/Entity.hpp"
#include "entities/NPC.hpp"
#include "entities/basic_plants_data.hpp"
#include "utils/TileMap.hpp"
#include "utils/collisionTypes.hpp"
#include "SceneManager.hpp"
#include "DialogueManager.hpp"
#include "states/FlagManager.hpp"
#include "utils/TextParser.hpp"
#include <functional>
#include "AudioManager.hpp"


struct Teleport {
    TileMap* layer;
    std::string targetMap;
    float spawnX, spawnY;
};

struct RenderItem {
    float zLayer;     // capa base (ground, entities, overhead…)
    float ySort;      // profundidad dentro de la capa
    std::function<void()> drawFunc;
};

class WorldState : public State {
    private:
        FlagManager* flagManager;
        DialogueManager dialogueManager;

        C3D_RenderTarget* top;
        
        std::string mapPath;
        float initialX, initialY;

        std::map<std::string, C2D_SpriteSheet> spriteSheets;
        int mapTilesWidth, mapTilesHeight;

        Hero* player;

        std::vector<TileMap*> allTileMaps;
        std::vector<Teleport> teleports; // Capa de teleports
        TileMap* collisionLayer = nullptr; // Capa de colisiones
        std::vector<TileMap*> layers; // Capas  de tiles
        
        std::vector<Object*> objetos; // Capa de objetos
        std::vector<NPC*> characters; // Capa de NPCs

        float camX = 0, camY = 0;

    public:
        WorldState(FlagManager* flagManager, C3D_RenderTarget* screen, std::string path, float startX, float startY);
        ~WorldState();

        void spawnObject(std::string sheetName, int idx, float x, float y, float z, int width, int height, int itemIndex);
        void loadNPC(const std::string& path); // cargar .txt con personaje
        void loadObject(const std::string& path); // cargar .txt con objetos
        void loadLevelFolder(const std::string& folderPath); // cargar carpeta con .csv y .txt (llama a crear layer o loadMap)

        void init() override;
        void update(float dt, u32 kDown) override;
        void draw() override;

        Entity* getInteractableEntity(float maxDistance);
        bool checkTeleportCollision(Teleport& outTp);
        bool checkLayerCollisions(TileMap* layer, float px, float py);
        CollisionType checkAllCollisions();

        C2D_SpriteSheet getSheet(std::string name) { return spriteSheets[name]; }

        DialogueManager& getDialogueManager() { return dialogueManager; }

};
#include "states/WorldState.hpp"

WorldState::WorldState(FlagManager* flagManager, C3D_RenderTarget* screen, std::string path, float startX, float startY)
    : flagManager(flagManager), dialogueManager(flagManager), top(screen), mapPath(path), initialX(startX), initialY(startY) {}

// GENERAR UN OBJETO
void WorldState::spawnObject(std::string sheetName, int idx, float x, float y, float z, int width, int height, int itemIndex) {
    if (spriteSheets.count(sheetName)) {
        Object* obj = new Object(x, y, z, width, height, idx, itemIndex, flagManager);
        obj->setSpriteSheet(spriteSheets[sheetName]);
        obj->init();

        objetos.push_back(obj);
    } else {
        //printf("ERROR: No existe el sheet %s\n", sheetName.c_str());
    }
}

// CARGAR MAPA PERSONAJE (.TXT)
void WorldState::loadNPC(const std::string& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        return;
    }

    std::string line;
    std::string npcName;
    std::string spriteName = "hero"; // default
    float x = 0, y = 0; // default
    float npcWidth = 0.0f, npcHeight = 0.0f;
    int spriteIdx, animate = 0;

    enum Parser_STATE { NONE, IN_BRANCH, IN_SAY, IN_SET, IN_CHOICE };
    Parser_STATE state = NONE;

    DialogueBranch currentBranch;
    std::vector<DialogueBranch> parsedBranches;
    DialogueChoice currentChoice;
    std::vector<std::string> currentLines;
    std::vector<std::pair<std::string, bool>> currentFlagsOnEnd;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string word;
        ss >> word;

        // --- NPC BASE ---
        if (word == "npc") {
            ss >> npcName;
        }
        else if (word == "position") {
            ss >> x >> y;
        }
        else if (word == "size") {
            ss >> npcWidth >> npcHeight;
        }
        else if (word == "sprite") {
            ss >> spriteName >> spriteIdx >> animate;
        }

        // --- BRANCH ---
        else if (word == "branch") {
            state = IN_BRANCH;
            currentBranch = {};
            currentLines.clear();
            currentFlagsOnEnd.clear();
        }
        else if (word == "when") {
            std::string flag;
            ss >> flag;

            if (flag == "none") {
                currentBranch.conditionFlag = "";
            } else {
                std::string valStr;
                ss >> valStr;
                bool val = (valStr == "true");

                currentBranch.conditionFlag = flag;
                currentBranch.expectedValue = val;
                
                // Iniciar flag si no existe a false
                if (flagManager->getCurrentFlags().count(flag) == 0)
                    flagManager->setFlag(flag, false);
            }
        }

        // --- SAY ---
        else if (word == "say") {
            state = IN_SAY;
        }
        else if (word == "choice") {
            state = IN_CHOICE;
            currentChoice = {};

            std::getline(ss, currentChoice.text);
            TextParser tp;
            currentChoice.text = tp.trim(currentChoice.text);
        }
        else if (word == "set" && state != IN_CHOICE) {
            state = IN_SET;
        }
        else if (word == "end") {
            if (state == IN_SAY) {
                state = IN_BRANCH;
            }
            else if (state == IN_SET) {
                state = IN_BRANCH;
            }
            else if (state == IN_CHOICE) {
                currentBranch.choices.push_back(currentChoice);
                currentBranch.hasChoices = true;
                state = IN_BRANCH;
            }
            else if (state == IN_BRANCH) {
                // cerrar branch
                currentBranch.lines = currentLines;
                currentBranch.setFlagsOnEnd = currentFlagsOnEnd;

                parsedBranches.push_back(currentBranch);

                state = NONE;
            }
        }

        // --- CONTENIDO ---
        else {
            TextParser tp;

            if (state == IN_SAY) {
                currentLines.push_back(tp.unescape(tp.trim(line)));
            }
            else if (state == IN_SET) {
                std::string flag = word;
                std::string valStr;
                ss >> valStr;
                bool val = (valStr == "true");
                currentFlagsOnEnd.push_back({tp.trim(flag), val});
            }
            else if (state == IN_CHOICE) {
                if (word == "set") {
                    std::string flag, valStr;
                    ss >> flag >> valStr;

                    bool val = (valStr == "true");
                    currentChoice.flagsOnSelect.push_back({flag, val});

                    if (flagManager->getCurrentFlags().count(flag) == 0) {
                        flagManager->setFlag(flag, false);
                    }
                }
            }
        }
    }

    // Añadir elementos al NPC
    NPC* npc = new NPC(x, y, npcWidth, npcHeight, npcName, flagManager, animate, spriteIdx);
    for (auto& b : parsedBranches) {
        npc->addBranch(b);
    }

    npc->init();
    npc->setSolid(true);

    if (spriteSheets.count(spriteName)) {
        npc->setSpriteSheet(spriteSheets[spriteName]);
    }

    characters.push_back(npc);
}

// CARGAR MAPA OBJETOS (.TXT):
void WorldState::loadObject(const std::string& path) {
    std::ifstream file(path);
    std::string sheetName;
    int spriteIdx;
    bool isSolid;
    float x, y, z, width, height;
    std::string flag;
    int itemIndex;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        ss >> sheetName >> spriteIdx >> x >> y >> z >> width >> height >> isSolid >> flag >> itemIndex;

        // Gestión flag
        if (flagManager->getCurrentFlags().count(flag) == 0) {
            flagManager->setFlag(flag, false); // El objeto no existía previamente: Agregamos su flag
        } 
        else if (flagManager->getFlag(flag) == true) {
            return; // El objeto existía y fue recogido: No lo volvemos a agregar
        }

        // Spawnear objeto
        spawnObject(sheetName, spriteIdx, x, y, z, width, height, itemIndex);
        if (isSolid) objetos.back()->setSolid(true);
        objetos.back()->setFlag(flag);
    }
}

// CARGAR CARPETA (con TILESET y PERSONAJES/OBJETOS).
void WorldState::loadLevelFolder(const std::string& folderPath) {
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) return;

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        std::string fileName = ent->d_name;
        std::string fullPath = folderPath + "/" + fileName;


        if (fileName.find(".csv") != std::string::npos) {
            float z = 0.1f;

            if (fileName.find("prota") != std::string::npos) continue;
            if (fileName.find("ground") != std::string::npos) z = 0.1f;
            if (fileName.find("sub1") != std::string::npos) z = 0.20f;
            if (fileName.find("sub2") != std::string::npos) z = 0.22f;
            if (fileName.find("sub3") != std::string::npos) z = 0.25f;
            if (fileName.find("furniture") != std::string::npos) z = 0.3f;
            if (fileName.find("top") != std::string::npos) z = 0.6f;

            C2D_SpriteSheet sheet = spriteSheets["tiles"];
            bool setDynamic = false;

            if (fileName.find("furniture") != std::string::npos) {
                sheet = spriteSheets["furniture"];                
            }

            TileMap* layer = new TileMap(sheet, z);
            layer->setDynamic(setDynamic);

            layer->loadFromCSV(fullPath);

            if (fileName.find("tpPrev") != std::string::npos) { layer->setSolid(TP_PREV); tpPrev = layer; }
            else if (fileName.find("tpNext") != std::string::npos) { layer->setSolid(TP_NEXT); tpNext = layer; }
            else if (fileName.find("collision") != std::string::npos) { layer->setSolid(COLLISION); collisionLayer = layer; }
            else {
                layers.push_back(layer);
            }
        }
        else if (fileName == "info.txt") {
            std::ifstream infoFile(fullPath);
            std::string key;
            while (infoFile >> key) {
                if (key == "next_map") infoFile >> nextMapPath;
                else if (key == "next_spawn") infoFile >> nextX >> nextY;
                else if (key == "prev_map") infoFile >> prevMapPath;
                else if (key == "prev_spawn") infoFile >> prevX >> prevY;
                else if (key == "size") infoFile >> mapTilesWidth >> mapTilesHeight;
            }
        }
        else if (fileName == "objects.txt") {
            loadObject(fullPath);
        }
        else if (fileName.find(".npc") != std::string::npos) {
            loadNPC(fullPath);
        }
    }

    closedir(dir);
}

// CREACIÓN DEL MAPA:
void WorldState::init() {

    spriteSheets["hero"] = C2D_SpriteSheetLoad("romfs:/gfx/hero.t3x");
    spriteSheets["basic_plants"] = C2D_SpriteSheetLoad("romfs:/gfx/basic_plants.t3x");
    spriteSheets["tiles"] = C2D_SpriteSheetLoad("romfs:/gfx/tileset.t3x");
    spriteSheets["furniture"] = C2D_SpriteSheetLoad("romfs:/gfx/furniture.t3x");
    spriteSheets["characters"] = C2D_SpriteSheetLoad("romfs:/gfx/characters.t3x");
    
    player = new Hero(initialX, initialY, 0.4f, flagManager);
    player->setSpriteSheet(spriteSheets["hero"]);
    player->init();

    loadLevelFolder(mapPath);
    std::sort(layers.begin(), layers.end(), [](TileMap* a, TileMap* b) {
        return a->getZ() < b->getZ();
    });

    camX = player->getX();
    camY = player->getY();
}

// CHECK COLISIONES: 0=NoColision, 1=Colision, 2=TP_Prev, 3=TP_Next
bool WorldState::checkLayerCollisions(TileMap* layer, float px, float py) {
    if (!layer) return false;

    // Usamos los offsets calculados en el Hero
    float left   = px + player->getOffsetX();
    float right  = px + player->getOffsetX() + player->getColWidth();
    float top    = py + player->getOffsetY();
    float bottom = py + player->getOffsetY() + player->getColHeight();

    // El -1.0f es vital para no "atascarse" en paredes paralelas
    return (layer->isSolidAt(left, top) || 
            layer->isSolidAt(right - 1.0f, top) || 
            layer->isSolidAt(left, bottom - 1.0f) || 
            layer->isSolidAt(right - 1.0f, bottom - 1.0f));
}

CollisionType WorldState::checkAllCollisions() {
    float px = player->getX();
    float py = player->getY();

    if (checkLayerCollisions(collisionLayer, px, py)) return COLLISION;
    if (checkLayerCollisions(tpPrev, px, py)) return TP_PREV;
    if (checkLayerCollisions(tpNext, px, py)) return TP_NEXT;

    for(Object* obj : objetos) {
        if (obj->isSolid() && player->checkCollision(*obj)) {
            return COLLISION;
        }
    }

    for(NPC* npc : characters) {
        if (npc->isSolid() && player->checkCollision(*npc)) {
            return COLLISION;
        }
    }

    return NO_COLLISION;
}

// DETECTAR "Interacción" ENTRE PERSONAJE-ENTITY
Entity* WorldState::getInteractableEntity(float maxDistance) {    
    // Centro del jugador
    float px = player->getCenterX();
    float py = player->getCenterY();

    float maxDist2 = maxDistance * maxDistance;

    Entity* closestEntity = nullptr;

    float closestDist2 = maxDist2;

    for (NPC* npc : characters) {
        if (!npc) continue;

        // Centro del NPC
        float nx = npc->getCenterX();
        float ny = npc->getCenterY();

        float dx = px - nx;
        float dy = py - ny;

        float dist2 = dx * dx + dy * dy;

        if (dist2 < closestDist2) {
            closestDist2 = dist2;
            closestEntity = npc;
        }
    }

    for (Object* obj : objetos) {
        if (!obj) continue;

        // Centro del Objeto
        float nx = obj->getCenterX();
        float ny = obj->getCenterY();

        float dx = px - nx;
        float dy = py - ny;

        float dist2 = dx * dx + dy * dy;

        if (dist2 < closestDist2) {
            closestDist2 = dist2;
            closestEntity = obj;
        }
    }

    return closestEntity;
}

// ACTUALIZACIÓN (FRAME):
void WorldState::update(float dt, u32 kDown) {
    float oldX = player->getX();
    float oldY = player->getY();

    // --- Movimiento personajes/objetos ---
    for(NPC* npc : characters) {
        npc->update(dt);
        npc->updateY(oldY);
    }

    for(Object* obj : objetos) {
        obj->update(dt);
    }

    // --- Diálogos ---
    if (dialogueManager.isActive()) {
        dialogueManager.update(dt, kDown);
        return;
    }

    // -- Interacción ---
    if (kDown & KEY_A) {
        Entity* ent = getInteractableEntity(Config::INTERACTION_DISTANTE);

        if (ent) {
            InteractionContext ctx;
            ctx.dialogueManager = &dialogueManager;
            ctx.inventory = manager->getInventory();

            ent->onInteract(ctx);

            if (ent->isPendingRemoval()) {
                auto it = std::find(objetos.begin(), objetos.end(), static_cast<Object*>(ent));
                if (it != objetos.end()) objetos.erase(it);
                delete ent;
            }
        }
    }

    // --- Movimiento personaje ---
    player->update(dt);

    // --- Movimiento en X ---
    player->moveX(dt);
    switch (checkAllCollisions()) {
        case COLLISION:
            player->setX(oldX);
            break;
        case TP_NEXT:
            if (!nextMapPath.empty()) {
                manager->changeState(new WorldState(
                    flagManager, top,
                    nextMapPath, nextX * Config::TILE_SIZE, nextY * Config::TILE_SIZE));
                return; 
            }
            break;
        case TP_PREV:
            if (!prevMapPath.empty()) {
                manager->changeState(new WorldState(
                    flagManager, top,
                    prevMapPath, prevX * Config::TILE_SIZE, prevY * Config::TILE_SIZE));
                return;
            }
            break;
        default:
            break;
    }

    // --- Movimiento en Y ---
    player->moveY(dt);
    switch (checkAllCollisions()) {
        case COLLISION:
            player->setY(oldY);
            break;
        case TP_NEXT:
            if (!nextMapPath.empty()) {
                manager->changeState(new WorldState(
                    flagManager, top,
                    nextMapPath, nextX * Config::TILE_SIZE, nextY * Config::TILE_SIZE));
                return; 
            }
            break;
        case TP_PREV:
            if (!prevMapPath.empty()) {
                manager->changeState(new WorldState(
                    flagManager, top,
                    prevMapPath, prevX * Config::TILE_SIZE, prevY * Config::TILE_SIZE));
                return;
            }
            break;
        default:
            break;
    }

    // --- ACTUALIZAR CÁMARA ---
    const float screenW_world = 400.0f / Config::globalScale;
    const float screenH_world = 240.0f / Config::globalScale;

    // Centrar por el centro del héroe:
    float heroCenterX = player->getX() + player->getWidth() / 2.0f;
    float heroCenterY = player->getY() + player->getHeight() / 2.0f;

    camX = heroCenterX - screenW_world / 2.0f;
    camY = heroCenterY - screenH_world / 2.0f;

    // Tamaño del mapa en mundo (NO escalado)
    float mapaAncho = mapTilesWidth * Config::TILE_SIZE;
    float mapaAlto  = mapTilesHeight * Config::TILE_SIZE;

    if (camX < 0) camX = 0;
    if (camY < 0) camY = 0;

    if (camX > mapaAncho - screenW_world) camX = mapaAncho - screenW_world;
    if (camY > mapaAlto - screenH_world) camY = mapaAlto - screenH_world;

    //printf("\nCam: %f, %f\nX, Y: %f, %f\n", camX, camY, player->getX(), player->getY());
}

// DIBUJAR:
void WorldState::draw() {
    C2D_TargetClear(top, C2D_Color32(0, 0, 50, 255));
    C2D_SceneBegin(top);

    for(TileMap* layer : layers) layer->draw(camX, camY, player->getX());
    
    if (collisionLayer && Config::showColissions) collisionLayer->draw(camX, camY, player->getX());
    if (tpNext && Config::showColissions) tpNext->draw(camX, camY, player->getX());
    if (tpPrev && Config::showColissions) tpPrev->draw(camX, camY, player->getX());

    for(Object* obj : objetos) obj->draw(camX, camY);
    for(NPC* npc : characters) npc->draw(camX, camY);
    
    player->draw(camX, camY);

    dialogueManager.draw();

    if(!dialogueManager.isActive()) {
        Entity* ent = getInteractableEntity(Config::INTERACTION_DISTANTE);
        if (ent) {
            dialogueManager.call_expression(ent, camX, camY);
        }
    }
}


// ELIMINAR (LIBERAR RECURSOS)
WorldState::~WorldState() {
    for(Object* obj : objetos) delete obj;
    objetos.clear();

    for(TileMap* layer : layers) delete layer;
    layers.clear();

    if (tpPrev) delete tpPrev;
    if (tpNext) delete tpNext;
    if (collisionLayer) delete collisionLayer;

    if (player) delete player;

    for (auto& pair : spriteSheets) {
        if (pair.second) {
            C2D_SpriteSheetFree(pair.second);
        }
    }
    spriteSheets.clear();
}
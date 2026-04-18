#include "states/WorldState.hpp"

#define DBG(fmt, ...) do { \
    char __buf[256]; \
    snprintf(__buf, sizeof(__buf), fmt, ##__VA_ARGS__); \
    dialogueManager.debug(__buf); \
} while (0)

#define DBG_MARK(tag) DBG("MARK: %s", tag)

WorldState::WorldState(FlagManager* flagManager,
    C3D_RenderTarget* screen, std::string path,
    float startX, float startY)
    : flagManager(flagManager),
      dialogueManager(flagManager),
      top(screen),
      mapPath(path),
      initialX(startX),
      initialY(startY),
      player(nullptr),
      collisionLayer(nullptr),
      camX(0.0f),
      camY(0.0f) {}

// GENERAR UN OBJETO
void WorldState::spawnObject(std::string sheetName, int idx, float x, float y, float z, int width, int height, int itemIndex) {
    if (spriteSheets.count(sheetName)) {
        Object* obj = new Object(x*Config::TILE_SIZE, y*Config::TILE_SIZE, z, width, height, idx, itemIndex, flagManager);
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
    NPC* npc = new NPC(x*Config::TILE_SIZE, y*Config::TILE_SIZE,
                    npcWidth, npcHeight,
                    npcWidth, npcHeight/2,
                    npcName, flagManager,
                    animate, spriteIdx);
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
        size_t prevSize = objetos.size();
        spawnObject(sheetName, spriteIdx, x, y, z, width, height, itemIndex);
        if (objetos.size() > prevSize) {
            Object* obj = objetos.back();
            if (isSolid) obj->setSolid(true);
            obj->setFlag(flag);
        }
    }
}

// CARGAR CARPETA (con TILESET y PERSONAJES/OBJETOS).
void WorldState::loadLevelFolder(const std::string& folderPath) {
    // Limpieza inicial
    collisionLayer = nullptr;
    teleports.clear();
    layers.clear();
    objetos.clear();
    characters.clear();
    allTileMaps.clear();

    
    std::map<std::string, Teleport> teleportInfo;

    DIR* dir = opendir(folderPath.c_str());
    if (!dir) return;

    struct dirent* ent;
    // PRIMERA PASADA
    // --- INFO.TXT ---
    while((ent = readdir(dir)) != NULL) {
        std::string fileName = ent->d_name;

        if (fileName == "info.txt") {
            std::string fullPath = folderPath + "/" + fileName;
            std::ifstream infoFile(fullPath);
            
            std::string key;
            while (infoFile >> key) {
                if (key == "size") {
                    infoFile >> mapTilesWidth >> mapTilesHeight;
                }
                else if (key == "tp") {
                    Teleport newTp;
                    std::string fileName;

                    infoFile >> fileName >> newTp.targetMap >> newTp.spawnX >> newTp.spawnY;
                    
                    teleportInfo[fileName] = newTp;
                }
            }
        }
    }
    closedir(dir);
    
    // SEGUNDA PASADA
    dir = opendir(folderPath.c_str());
    if (!dir) return;

    while ((ent = readdir(dir)) != NULL) {
        std::string fileName = ent->d_name;
        std::string fullPath = folderPath + "/" + fileName;

        // --- OBJETOS.TXT ---
        if (fileName == "objects.txt") {
            loadObject(fullPath);
        }

        // --- .NPC ---
        else if (fileName.find(".npc") != std::string::npos) {
            loadNPC(fullPath);
        }

        // --- CSV ---
        else if (fileName.find(".csv") != std::string::npos) {
            
            C2D_SpriteSheet sheet = spriteSheets["tiles"];
            float z = 0.5f;

            // Colisiones
            if (fileName.find("collision") != std::string::npos) {
                TileMap* layer = new TileMap(sheet, z);
                layer->loadFromCSV(fullPath);
                layer->setSolid(COLLISION);
                collisionLayer = layer;
                allTileMaps.push_back(layer);
            }

            // Teleports
            else if (fileName.find("_tp") != std::string::npos) {
                TileMap* layer = new TileMap(sheet, z);
                layer->loadFromCSV(fullPath);
                layer->setSolid(TP);

                // Buscar configuración en info.txt
                if (teleportInfo.count(fileName)) {
                    Teleport tp = teleportInfo[fileName];
                    tp.layer = layer;
                    teleports.push_back(tp);
                    allTileMaps.push_back(layer);
                }
            }

            // Capas normales
            else {
                // Detectar altura (def. = 0.1f):
                if (fileName.find("debug") != std::string::npos) continue;
                if (fileName.find("ground") != std::string::npos) z = 0.1f;
                if (fileName.find("sub1") != std::string::npos) z = 0.20f;
                if (fileName.find("sub2") != std::string::npos) z = 0.22f;
                if (fileName.find("sub3") != std::string::npos) z = 0.25f;
                if (fileName.find("furniture") != std::string::npos) z = 0.3f;
                if (fileName.find("top") != std::string::npos) z = 0.6f;

                // Detectar sheet (opcional)
                if (fileName.find("furniture") != std::string::npos) sheet = spriteSheets["furniture"];     
                if (fileName.find("other") != std::string::npos) {sheet = spriteSheets["other"]; z+=0.01f;} 
                if (fileName.find("efecto") != std::string::npos) {sheet = spriteSheets["efectos"]; z+=0.02f;}
                if (fileName.find("ruinas") != std::string::npos) {sheet = spriteSheets["ruinas"]; z+=0.03f;}

                TileMap* layer = new TileMap(sheet, z);
                layer->loadFromCSV(fullPath);
                layers.push_back(layer);
                allTileMaps.push_back(layer);
            }
        }
    }

    closedir(dir);
}

// CREACIÓN DEL MAPA:
void WorldState::init() {
    spriteSheets["hero"]        = ResourceManager::get().get("hero");
    spriteSheets["basic_plants"]= ResourceManager::get().get("basic_plants");
    spriteSheets["tiles"]       = ResourceManager::get().get("tiles");
    spriteSheets["other"]       = ResourceManager::get().get("other");
    spriteSheets["ruinas"]      = ResourceManager::get().get("ruinas");
    spriteSheets["efectos"]     = ResourceManager::get().get("efectos");
    spriteSheets["furniture"]   = ResourceManager::get().get("furniture");
    spriteSheets["characters"]  = ResourceManager::get().get("characters");


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

// CHECK COLISIONES: 0=NoColision, 1=Colision
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

bool WorldState::checkTeleportCollision(Teleport& outTp) {
    float px = player->getX();
    float py = player->getY();

    for (auto& tp : teleports) {
        if (checkLayerCollisions(tp.layer, px, py)) {
            outTp = tp;
            return true;
        }
    }
    return false;
}

CollisionType WorldState::checkAllCollisions() {
    float px = player->getX();
    float py = player->getY();

    if (checkLayerCollisions(collisionLayer, px, py)) return COLLISION;
    
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
    // ===================== DEBUG EXTENSO =====================
    {
        dialogueManager.clearDebug();
    }

    if (!player) {
        DBG("ERROR: player NULL");
        return;
    }
    float oldX = player->getX();
    float oldY = player->getY();

    // --- Movimiento personajes/objetos ---
    for(NPC* npc : characters) {
        if (!npc) { DBG("NPC NULL!"); continue; }
        npc->update(dt);
        npc->updateY(oldY);
    }
    
    for (Object* obj : objetos) {
        if (!obj) { DBG("OBJ NULL!"); continue; }
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
            ctx.inventory = manager ? manager->getInventory() : nullptr;

            ent->onInteract(ctx);

            if (ent->isPendingRemoval()) {
                auto itObj = std::find_if(objetos.begin(), objetos.end(),
                    [ent](Object* obj) { return static_cast<Entity*>(obj) == ent; });
                if (itObj != objetos.end()) {
                    objetos.erase(itObj);
                } else {
                    auto itNpc = std::find_if(characters.begin(), characters.end(),
                        [ent](NPC* npc) { return static_cast<Entity*>(npc) == ent; });
                    if (itNpc != characters.end()) {
                        characters.erase(itNpc);
                    }
                }
                delete ent;
            }
        }
    }
    
    // --- Movimiento personaje ---
    player->update(dt);

    // --- Movimiento en X ---
    player->moveX(dt);
    if (checkAllCollisions() == COLLISION) {
        player->setX(oldX);
    }

    // --- Movimiento en Y ---
    player->moveY(dt);
    if (checkAllCollisions() == COLLISION) {
        player->setY(oldY);
    }
    
    // --- TP ---
    Teleport tp;
    if (checkTeleportCollision(tp)) {
        DBG("TP_HIT: layer=%p target=%s\nspawn=(%.1f,%.1f)",
            tp.layer, tp.targetMap.c_str(), tp.spawnX, tp.spawnY);

        DBG_MARK("TP_BEFORE_CHANGE_STATE");
        WorldState* next = new WorldState(
            flagManager, top,
            tp.targetMap,
            tp.spawnX * Config::TILE_SIZE,
            tp.spawnY * Config::TILE_SIZE
        );

        DBG("TP NEW WS=%p", next);

        manager->changeState(next);

        DBG("TP AFTER changeState (this=%p)", this);
        return;
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
    
}

// DIBUJAR:
void WorldState::draw() {
    DBG_MARK("DRAW_BEGIN");
    std::vector<RenderItem> renderList;
    for (TileMap* layer : layers) {
        renderList.push_back({
            layer->getZ(),
            0.0f, // no importa dentro de la capa
            [=]() { layer->draw(camX, camY, 0); }
        });
    }

    for (Entity* e : characters) {
        renderList.push_back({
            0.5f, // capa entidades
            e->getY() + e->getHeight(),
            [=]() { e->draw(camX, camY); }
        });
    }

    for (Entity* e : objetos) {
        renderList.push_back({
            0.5f,
            e->getY() + e->getHeight(),
            [=]() { e->draw(camX, camY); }
        });
    }

    renderList.push_back({
        0.5f,
        player->getY() + player->getHeight(),
        [=]() { player->draw(camX, camY); }
    });

    std::sort(renderList.begin(), renderList.end(),
        [](const RenderItem& a, const RenderItem& b) {
            if (a.zLayer == b.zLayer)
                return a.ySort < b.ySort;
            return a.zLayer < b.zLayer;
    });

    DBG_MARK("DRAW_BEFORE_SCENEBEGIN");
    C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
    C2D_SceneBegin(top);

    for (auto& item : renderList) {
        item.drawFunc();
    }
    DBG_MARK("DRAW_AFTER_RENDERLIST");

    if (Config::showColissions) {
        DBG_MARK("DRAW_COLLISIONS");
        if (collisionLayer) collisionLayer->draw(camX, camY, player->getX());
        for (auto& tp : teleports) {
            if (!tp.layer) {
                DBG("WARN: TP layer NULL in draw");
                continue;
            }
            tp.layer->draw(camX, camY, player->getX());
        }
    }

    DBG_MARK("DRAW_BEFORE_DIALOGUE");
    dialogueManager.draw();
    DBG_MARK("DRAW_AFTER_DIALOGUE");

    if(!dialogueManager.isActive()) {
        DBG_MARK("DRAW_BEFORE_INTERACTABLE");
        Entity* ent = getInteractableEntity(Config::INTERACTION_DISTANTE);
        if (ent) {
            dialogueManager.call_expression(ent, camX, camY);
        }
        DBG_MARK("DRAW_AFTER_INTERACTABLE");
    }

    dialogueManager.drawDebug();
}


// ELIMINAR (LIBERAR RECURSOS)
WorldState::~WorldState() {
    for(NPC* npc : characters) delete npc;
    characters.clear();

    for(Object* obj : objetos) delete obj;
    objetos.clear();

    for(TileMap* layer : allTileMaps) delete layer;
    allTileMaps.clear();

    layers.clear();
    teleports.clear();
    collisionLayer = nullptr;

    if (player) delete player;

    spriteSheets.clear();
}
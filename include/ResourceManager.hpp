#pragma once
#include <map>
#include <string>
#include <citro2d.h>

class ResourceManager {
public:
    static ResourceManager& get() {
        static ResourceManager instance;
        return instance;
    }

    void init() {
        sheets["hero"]        = C2D_SpriteSheetLoad("romfs:/gfx/hero.t3x");
        sheets["basic_plants"]= C2D_SpriteSheetLoad("romfs:/gfx/basic_plants.t3x");
        sheets["tiles"]       = C2D_SpriteSheetLoad("romfs:/gfx/tileset.t3x");
        sheets["other"]       = C2D_SpriteSheetLoad("romfs:/gfx/tileset_other.t3x");
        sheets["ruinas"]      = C2D_SpriteSheetLoad("romfs:/gfx/ruinas.t3x");
        sheets["efectos"]     = C2D_SpriteSheetLoad("romfs:/gfx/efectos.t3x");
        sheets["furniture"]   = C2D_SpriteSheetLoad("romfs:/gfx/furniture.t3x");
        sheets["characters"]  = C2D_SpriteSheetLoad("romfs:/gfx/characters.t3x");
        sheets["dialogue"]    = C2D_SpriteSheetLoad("romfs:/gfx/dialogue.t3x");
        sheets["inventory"]   = C2D_SpriteSheetLoad("romfs:/gfx/inventory.t3x");
    }

    void exit() {
        for (auto& p : sheets) {
            if (p.second) C2D_SpriteSheetFree(p.second);
        }
        sheets.clear();
    }

    C2D_SpriteSheet get(const std::string& name) {
        return sheets[name];
    }

private:
    std::map<std::string, C2D_SpriteSheet> sheets;
    ResourceManager() = default;
};

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>

#include "SceneManager.hpp"
#include "states/WorldState.hpp"
#include "states/FlagManager.hpp"
#include "utils/Config.hpp"
#include "AudioManager.hpp"
#include "ResourceManager.hpp"

// Iniciar RomFS para poder leer archivos de la SD
bool romfsInit_Seguro() {
    Result rc = romfsInit();
    if (R_FAILED(rc)) {
        return false;
    }
    return true;
}


int main()
{
    gfxInitDefault();
    if(!romfsInit_Seguro()) return 0;

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    cfguInit();
    C2D_Prepare();
    
    C3D_RenderTarget* topTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // MANAGERS GLOBALES:
    FlagManager flagManager;
    ResourceManager::get().init();
    AudioManager::get().init();
    
    // Inicio personaje:
    float initialX = 10 * Config::TILE_SIZE;
    float initialY = 11 * Config::TILE_SIZE;
    SceneManager* manager = new SceneManager(topTarget, bottomTarget);
    manager->changeState(new WorldState(&flagManager, topTarget, "romfs:/data/casa_interior", initialX, initialY)); // Empezar en mundo
    //manager->changeState(new WorldState(&flagManager, topTarget, "romfs:/data/medieval", 3*Config::TILE_SIZE, 9*Config::TILE_SIZE)); // Empezar en medieval
    //manager->changeState(new WorldState(&flagManager, topTarget, "romfs:/data/playa", 22*Config::TILE_SIZE, 4*Config::TILE_SIZE)); // Empezar en playa
    //manager->changeState(new WorldState(&flagManager, topTarget, "romfs:/data/pueblo", 41*Config::TILE_SIZE, 33*Config::TILE_SIZE)); // Empezar en playa
    
    // Delta Time:
    u64 tiempoAnterior = osGetTime();

    while (aptMainLoop())
    {
        // Calcular retardo para velocidad
        u64 tiempoActual = osGetTime();
        float dt = (tiempoActual - tiempoAnterior) / 1000.0;
        tiempoAnterior = tiempoActual;

        // Leer entrada
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
		if (kDown & KEY_START) break; // Condición salida

        // DEBUG:
        if (kDown & KEY_X) {
            Config::showColissions = !Config::showColissions;
        }
        if (kHeld & KEY_R) {
            Config::globalScale += dt;
        }
        else if (kHeld & KEY_L) {
            Config::globalScale -= dt;
        }
        
        // Touch screen Input
		touchPosition touch;
		hidTouchRead(&touch);

        // CirclePad Input
        circlePosition pos;
		hidCircleRead(&pos);

        // APLICACIÓN
        manager->update(dt, kDown);
        AudioManager::get().update();

        // DRAW
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        manager->draw();
        C3D_FrameEnd(0);
    }

    // Limpieza
    delete manager;
    AudioManager::get().exit();
    ResourceManager::get().exit();
    romfsExit();
    cfguExit();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>

#include "SceneManager.hpp"
#include "states/WorldState.hpp"
#include "states/FlagManager.hpp"
#include "utils/Config.hpp"

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

    // FLAGS GLOBALES:
    FlagManager flagManager;

    // Inicio personaje:
    float initialX = 10 * Config::TILE_SIZE;
    float initialY = 11 * Config::TILE_SIZE;
    SceneManager* manager = new SceneManager(topTarget, bottomTarget);
    manager->changeState(new WorldState(&flagManager, topTarget, "romfs:/data/1_casa_interior", initialX, initialY)); // Empezar en mundo
    
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

        // DEBUG:
        /*
        printf("\x1b[1;1H\x1b[32m-- DEBUG CONSOLE --\x1b[0m"); // Título en verde
        printf("\x1b[2;1HCP: %04d; %04d\x1b[K", pos.dx, pos.dy); // \x1b[K limpia hasta el final de la línea
        printf("\x1b[3;1HTP: %03d; %03d\x1b[K", touch.px, touch.py);
        printf("\x1b[5;1HCPU:    %6.2f%%\x1b[K", C3D_GetProcessingTime()*6.0f);
        printf("\x1b[6;1HGPU:    %6.2f%%\x1b[K", C3D_GetDrawingTime()*6.0f);
        
        printf("\x1b[8;1H\x1b[33m-- FLAGS --\x1b[0m\x1b[K");
        
        // Dibujamos las flags a partir de la línea 9
        int line = 9;
        for (auto const& f : flagManager.getCurrentFlags()) {
            printf("\x1b[%d;1H %-15s | %s\x1b[K", line++, (f.first).c_str(), f.second ? "true" : "false");
            if(line > 25) break; // Evitar salirnos de la pantalla
        }
                
        // Limpiar las líneas sobrantes por si alguna flag se borra
        for(int i = line; i < 28; i++) printf("\x1b[%d;1H\x1b[K", i);
        */

        // APLICACIÓN
        manager->update(dt, kDown);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        manager->draw();
        C3D_FrameEnd(0);
    }

    // Limpieza
    delete manager;
    romfsExit();
    cfguExit();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
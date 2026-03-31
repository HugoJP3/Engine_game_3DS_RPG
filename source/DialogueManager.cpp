#include "DialogueManager.hpp"

DialogueManager::DialogueManager(FlagManager* flagManager) : flagManager(flagManager) {
    ui_dialogue = C2D_SpriteSheetLoad("romfs:/gfx/dialogue.t3x");
    
    text_Buffer = C2D_TextBufNew(4096);
    nombre_Buffer = C2D_TextBufNew(4096);

    active = false;
}

DialogueManager::~DialogueManager() {
    if (text_Buffer) C2D_TextBufDelete(text_Buffer);
    if (nombre_Buffer) C2D_TextBufDelete(nombre_Buffer);
    if (ui_dialogue) C2D_SpriteSheetFree(ui_dialogue);

}

void DialogueManager::startDialogue(const DialogueBranch dialogueBranch, std::string name) {
    if (dialogueBranch.lines.empty()) return;
    if (name.empty()) return;

    character_name = name;
    currentBranch = dialogueBranch;

    currentLineIdx = 0;
    charIdx = 0;

    inChoiceMode = false;
    selectedChoice = 0;

    active = true;
}


void DialogueManager::update(float dt, u32 kDown) {
    if (!active) return;

    // --- Lógica de elección múltiple ---
    if (inChoiceMode) {
        animTime += dt;

        if (kDown & KEY_UP) {
            selectedChoice--;
            if (selectedChoice < 0) selectedChoice = currentBranch.choices.size() - 1;
        }

        if (kDown & KEY_DOWN) {
            selectedChoice++;
            if (selectedChoice >= currentBranch.choices.size()) selectedChoice = 0;
        }

        if (kDown & KEY_A) {
            // aplicar flags de la opción
            for (auto& f : currentBranch.choices[selectedChoice].flagsOnSelect) {
                flagManager->setFlag(f.first, f.second);
            }

            active = false;
            inChoiceMode = false;
        }

        return; // Mientras haya elección, no se manipula el resto
    }

    // --- Lógica avanzar un caracter ---
    animFrame += dt;
    if (animFrame >= velWriting) {
        animFrame = 0.0f;
        
        // Avanzar frase solo si no hemos llegado al final de esta
        if (charIdx < currentBranch.lines[currentLineIdx].length()) {
            unsigned char c = (unsigned char)currentBranch.lines[currentLineIdx][charIdx];
            size_t res = 1;

            // Detectar cuántos bytes ocupa el carácter actual
            if (c >= 0xf0)      res = 4;
            else if (c >= 0xe0) res = 3;
            else if (c >= 0xc0) res = 2;
            else                res = 1;

            charIdx += res;

            // Seguridad: No pasarnos del final por si el archivo está mal formado
            if (charIdx > currentBranch.lines[currentLineIdx].length()) {
                charIdx = currentBranch.lines[currentLineIdx].length();
            }
        }
    }

    if (kDown & KEY_A) {
        if (charIdx < currentBranch.lines[currentLineIdx].length()) {
            charIdx = currentBranch.lines[currentLineIdx].length();
        } else {
            currentLineIdx++;
            charIdx = 0.0f;

            // Fin diálogo
            if (currentLineIdx >= currentBranch.lines.size()) {

                // Si hay elección, la activamos
                if (currentBranch.hasChoices) {
                    inChoiceMode = true;
                    selectedChoice = 0;
                    return;
                }

                active = false;

                // Activar flags post-diálogo
                for (auto& f : currentBranch.setFlagsOnEnd) {
                    flagManager->setFlag(f.first, f.second);
                }
            }
        }
    }
}

void DialogueManager::call_expression(float draw_x, float draw_y, Expression expression) {
    C2D_Image img = C2D_SpriteSheetGetImage(ui_dialogue, expression);
    C2D_DrawImageAt(img, draw_x, draw_y, 0.95f, NULL, Config::globalScale, Config::globalScale);
}

void DialogueManager::call_expression(Entity* ent, float camX, float camY) {
    Expression expression = ent->getInteractableExpression();

    // 1. Calculamos la posición relativa al centro del personaje en el mundo
    float mundoX = ent->getX() - camX + (ent->getWidth() / 2.0f);
    float mundoY = ent->getY() - camY;

    // 2. Convertimos a coordenadas de pantalla multiplicando por la escala
    float screenX = mundoX * Config::globalScale;
    float screenY = mundoY * Config::globalScale;

    // 3. Ajustamos el offset vertical para que flote sobre la cabeza (también escalado)
    screenY -= 10.0f * Config::globalScale; 
    
    // 4. Centramos el icono (si el icono mide 16px, restamos 8 * escala)
    screenX -= 2.5f * Config::globalScale;

    // Dibujamos
    C2D_Image img = C2D_SpriteSheetGetImage(ui_dialogue, expression);
    C2D_DrawImageAt(img, screenX, screenY, 0.95f, NULL, Config::globalScale * 1.25, Config::globalScale * 1.25f);
}


void DialogueManager::draw() {
    if (!active || !ui_dialogue || charIdx < 0) return;
    
    // --- RENDERIZADO DE TEXTO ---
    C2D_TextBufClear(text_Buffer);
    C2D_TextBufClear(nombre_Buffer);

    C2D_Text texto_mostrar, texto_nombre;
    C2D_TextParse(&texto_mostrar, text_Buffer, currentBranch.lines[currentLineIdx].substr(0, charIdx).c_str());
    C2D_TextParse(&texto_nombre, nombre_Buffer, character_name.c_str());
    
    C2D_TextOptimize(&texto_mostrar);
    C2D_TextOptimize(&texto_nombre);
    
    // --- COLORES ---
    u32 col_fondo_ext = C2D_Color32(30, 20, 40, 230);   // Púrpura muy oscuro (borde)
    u32 col_fondo_int = C2D_Color32(10, 10, 15, 250);   // Casi negro (fondo principal)
    u32 col_borde_lin = C2D_Color32(70, 45, 60, 255);   // Vino apagado para líneas finas
    
    u32 col_nombre    = C2D_Color32(200, 120, 80, 255); // Cobre/Naranja atardecer
    u32 col_texto     = C2D_Color32(220, 220, 200, 255); // Blanco hueso (relajante)
    u32 col_sombra    = C2D_Color32(0, 0, 0, 180);       // Sombra negra pura
    u32 col_sel       = C2D_Color32(100, 150, 200, 255); // Azul pálido (frío de la noche)

    // --- VARIABLES ---
    float boxWidth = 380.0f;
    float boxHeight = 80.0f;
    float x_box = (400.0f - boxWidth) / 2.0f;
    float y_box = (240.0f - boxHeight) - 10.0f;
    float margin_box = 2.0f;

    // --- DIBUJO CAJAS TEXTO ---
    // Sombra exterior
    C2D_DrawRectSolid(x_box - margin_box, y_box - margin_box, 0.85f, boxWidth + margin_box * 2.0f, boxHeight + margin_box * 2.0f , col_fondo_ext);
    // Fondo principal
    C2D_DrawRectSolid(x_box, y_box, 0.86f, boxWidth, boxHeight, col_fondo_int);    
    // Línea decorativa superior
    C2D_DrawRectSolid(x_box + (margin_box * 2.0f) + 1.0f, y_box + margin_box, 0.87f, boxWidth - 10, 1, col_borde_lin);
    
    // --- MOSTRAR TEXTOS ---
    float textNombreWidth, textNombreHeight;
    C2D_TextGetDimensions(&texto_nombre, 0.55f, 0.55f, &textNombreWidth, &textNombreHeight);

    float margenX_text = 8.0f;
    float margenY_text = 22.0f;
    float margenX_nombre = 8.0f;
    float margenY_nombre = 4.0f;
    float desplazamiento_Sombra = 1.0f;
    float tamTexto = 0.7f;

    // Texto + Sombra
    C2D_DrawText(&texto_mostrar, C2D_WithColor, x_box + margenX_text + desplazamiento_Sombra, y_box + margenY_text + desplazamiento_Sombra, 0.98f, tamTexto, tamTexto, col_sombra);
    C2D_DrawText(&texto_mostrar, C2D_WithColor, x_box + margenX_text, y_box + margenY_text, 1.0f, tamTexto, tamTexto, col_texto);

    // Nombre + Sombra
    C2D_DrawText(&texto_nombre, C2D_WithColor, x_box + margenX_nombre + desplazamiento_Sombra, y_box + margenY_nombre + desplazamiento_Sombra, 0.98f,tamTexto, tamTexto, col_sombra);
    C2D_DrawText(&texto_nombre, C2D_WithColor, x_box + margenX_nombre, y_box + margenY_nombre, 1.0f, tamTexto, tamTexto, col_nombre);

    // Triángulo
    if (charIdx >= currentBranch.lines[currentLineIdx].length()) {
        if (((int)(osGetTime() / 500) % 2) == 0) {
            float triX = x_box + boxWidth - 22.0f;
            float triY = y_box + boxHeight - 18.0f;

            C2D_Image img_triangle = C2D_SpriteSheetGetImage(ui_dialogue, 2);
            C2D_DrawImageAt(img_triangle, triX, triY, 1.0f, NULL, 0.75f, 0.75f);
        }
    }

    // --- MOSTRAR ELECCIONES MÚLTIPLES ---
    if (inChoiceMode) {
        float spacing = 22.0f;
        float marginY = (boxHeight / currentBranch.choices.size()) / 2.0f;
        float startChoiceY = y_box + marginY;

        C2D_TextBufClear(text_Buffer);
        for (size_t i = 0; i < currentBranch.choices.size(); i++) {
            C2D_Text txt;
            C2D_TextParse(&txt, text_Buffer, currentBranch.choices[i].text.c_str());
            C2D_TextOptimize(&txt);

            float offsetX = 0.0f;
            float offsetY = 0.0f;

            u32 current_col = col_texto;

            if (i == selectedChoice) {
                current_col = col_sel;
                // Efecto "Wave"
                offsetX = sinf(animTime * 6.0f) * 2.0f;
                offsetY = cosf(animTime * 3.0f) * 1.0f;
                
                // Indicador retro ">"
                C2D_Text marker;
                C2D_TextParse(&marker, text_Buffer, ">");

                float markerOffset = sinf(animTime * 8.0f) * 3.0f;
                C2D_DrawText(&marker, C2D_WithColor, x_box + 10 + markerOffset + offsetX, startChoiceY + (i * spacing), 1.0f, 0.6f, 0.6f, col_texto);
                offsetX += 15.0f; 
            }

            C2D_DrawText(&txt, C2D_WithColor,
                x_box + 20 + offsetX,      // Aplicamos el meneo en X
                startChoiceY + (i * spacing) + offsetY, // Aplicamos el meneo en Y
                1.0f, tamTexto, tamTexto, current_col);
        }
    }
}
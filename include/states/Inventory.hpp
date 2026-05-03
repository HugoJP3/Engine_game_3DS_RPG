#pragma once
#include <vector>
#include <algorithm>
#include <citro2d.h>
#include "ResourceManager.hpp"

struct Item {
    int ItemIndex;
};

class Inventory {
    private:
        std::vector<Item> inventario; // Guarda los iconIndex de los objetos recogidos
        C3D_RenderTarget* bottom;
        C2D_SpriteSheet itemSheets;

    public:
        Inventory(C3D_RenderTarget* target) : bottom(target) {
            itemSheets = ResourceManager::get().get("objects");
        }

        ~Inventory() {}

        // Añade un item al inventario
        void addItemIndex(int itIdx) {
            for (size_t i = 0; i < inventario.size(); i++) {
                if (inventario[i].ItemIndex == itIdx) {
                    return;
                }
            }

            // Añade el item si no estaba
            Item newItem = {itIdx};

            inventario.push_back(newItem);
        }

        // Trata de eliminar un item del inventario, false si no existe
        bool substractItemIndex(int itIdx) {
            for (auto it = inventario.begin(); it != inventario.end(); it++) {
                if (it->ItemIndex == itIdx) {
                    inventario.erase(it);
                    return true;
                }
            }
            return false;
        }

        void draw() {
            C2D_SceneBegin(bottom);
            C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));

            // Dibujado UI básica
            C2D_Image ui = C2D_SpriteSheetGetImage(itemSheets, 0);
            C2D_DrawImageAt(ui, 0, 0, 0.1f);

            // Dibujado objetos
            float startX = 21.0f;
            float startY = 50.5f;
            float spacingX = 49.0f; // 48-50 // Espacio entre iconos
            float spacingY = 52.5f; // 52-53 Espacio entre iconos
            int itemsPerRow = 6;

            for (size_t i = 0; i < inventario.size(); i++) {
                float x = startX + (i % itemsPerRow) * spacingX;
                float y = startY + (i / itemsPerRow) * spacingY;

                // Dibujar el icono del item
                C2D_Image img = C2D_SpriteSheetGetImage(itemSheets, inventario[i].ItemIndex);
                C2D_DrawImageAt(img, x, y, 0.6f, NULL, 2.0f, 2.0f); // Escala 2x para que se vea bien
            }
        }

};
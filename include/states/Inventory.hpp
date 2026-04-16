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
            itemSheets = ResourceManager::get().get("inventory");
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
            C2D_TargetClear(bottom, C2D_Color32(30, 30, 30, 255)); // Fondo gris oscuro

            float startX = 20.0f;
            float startY = 20.0f;
            float spacing = 48.0f; // Espacio entre iconos
            int itemsPerRow = 6;

            for (size_t i = 0; i < inventario.size(); i++) {
                float x = startX + (i % itemsPerRow) * spacing;
                float y = startY + (i / itemsPerRow) * spacing;

                // Dibujar un recuadro de fondo para el slot (sustituir por sprite)
                C2D_DrawRectSolid(x - 4, y - 4, 0.5f, 40, 40, C2D_Color32(60, 60, 60, 255));

                // Dibujar el icono del item
                C2D_Image img = C2D_SpriteSheetGetImage(itemSheets, inventario[i].ItemIndex);
                C2D_DrawImageAt(img, x, y, 0.6f, NULL, 2.0f, 2.0f); // Escala 2x para que se vea bien
            }
        }

};
// Características de un objeto genérico
#pragma once
#include <3ds.h>
#include "entities/Entity.hpp"
#include "states/Inventory.hpp"

class Object : public Entity {
    protected:
        int baseIndex;
        int itemIndex;

        std::string flagOnGet;
        
    public:
        Object(float x, float y, float z, int width, int height, int baseIndex, int itemIndex, FlagManager* flagManager);
        ~Object();
        
        void init() override;
        void update(float dt) override;
        void draw(float camX, float camY) override;

        int getItemIndex() const{ return itemIndex; }

        void onInteract(InteractionContext& ctx) override;

        void setFlag(std::string flag) { flagOnGet = flag; }
        void takeObject();
};
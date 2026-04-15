// Características de un objeto genérico
#include "entities/Object.hpp"

Object::Object(float x, float y, float z, int width, int height, int baseIndex, int itemIndex, FlagManager* flagManager)
    : Entity(x, y, z,
        width, height,
        width, height,
        flagManager),
        baseIndex(baseIndex), itemIndex(itemIndex) {}

Object::~Object() {}

// Para cargar un sprite propio
void Object::init() {}

void Object::update(float dt) {
    // Nada pasa, es un objeto
}

void Object::draw(float camX, float camY) {
    if (!spriteSheet) return;

    C2D_Image img = C2D_SpriteSheetGetImage(spriteSheet, baseIndex);
    
    C2D_DrawImageAt(img, getRenderX(camX), getRenderY(camY), z, NULL, Config::globalScale, Config::globalScale);

    if (Config::showColissions) {
        drawCollision(camX, camY);
    }
}

void Object::takeObject() {
    flagManager->setFlag(flagOnGet, true);
    Sound& object = AudioManager::get().getSound("romfs:/audio/object.wav");
    AudioManager::get().playSFX(object);
}

void Object::onInteract(InteractionContext& ctx) {
    this->takeObject();

    if (ctx.inventory) {
        ctx.inventory->addItemIndex(this->getItemIndex());
    }

    this->setPendingRemoval(true);
}
#ifndef COLLISION_TYPE_H
#define COLLISION_TYPE_H

// Estados de dirección para la lógica
typedef enum {
    NO_COLLISION=0,
    COLLISION,
    TP_PREV,
    TP_NEXT,
} CollisionType;

#endif
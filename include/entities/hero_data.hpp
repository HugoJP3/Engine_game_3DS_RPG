#ifndef HERO_H
#define HERO_H

// Índices de los sprites según el orden en el .t3s
enum {
    // Fila 0: Frente
    SPR_HERO_FRONT_IDLE = 0,
    SPR_HERO_FRONT_WALK1,
    SPR_HERO_FRONT_WALK2,
    //SPR_HERO_FRONT_WALK3,

    // Fila 1: Espalda
    SPR_HERO_BACK_IDLE,
    SPR_HERO_BACK_WALK1,
    SPR_HERO_BACK_WALK2,
    //SPR_HERO_BACK_WALK3,

    // Fila 2: Perfil Izquierdo
    SPR_HERO_LEFT_IDLE,
    SPR_HERO_LEFT_WALK1,
    SPR_HERO_LEFT_WALK2,
    //SPR_HERO_LEFT_WALK3,

    // Fila 3: Perfil Derecho
    SPR_HERO_RIGHT_IDLE,
    SPR_HERO_RIGHT_WALK1,
    SPR_HERO_RIGHT_WALK2,
    //SPR_HERO_RIGHT_WALK3
};

// Estados de dirección para la lógica
typedef enum {
    DIR_FRONT,
    DIR_SIDE_LEFT,
    DIR_BACK,
    DIR_SIDE_RIGHT,
} HeroDirection;

#endif
objetct.txt:::
TIPO SpriteID X Y Z Solid flag itemIdx

dialogos.txt:::
    [ID_UNICO]
    texto.1
    ...
    text.next_map
    [EVENT: START_MINIGAME_MATH]
    [END]

levelX/
    ground.csv
    decor.csv
    top.csv
    collision.csv = Colisiones invisibles
    tpNext.csv
    tpPrev.csv
    objects.txt
    info.txt

info.txt:::
next_map romfs:/data/level_Pruebas
next_spawn 300 300
prev_map romfs:/data/level_Pruebas
prev_spawn 400 160


personaje.npc.txt:::
mucha cosa jaja


CAPAS:::
    collision/tp    = 0.1
    ground          = 0.1
    sub1...subn           = 0.2..0.2n
    furniture       = 0.25
    objects         = 0.3
    hero            = 0.4
    npc = dinámico con hero = 0.35 a 0.45
    top             = 0.6
    colisiones visibles = 0.8
    diálogo         = 0.8 a 1.0

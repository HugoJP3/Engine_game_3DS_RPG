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
    sub1...subn.csv
    furniture.csv
    other.csv
    top.csv
    collision.csv = Colisiones invisibles
    _tp.csv ...
    objects.txt
    info.txt

info.txt:::
tp <nombre> <mapa_destino> <spawnX> <spawnY>

personaje.npc.txt:::
mucha cosa

CAPAS:::
    collision/tp    = 0.1
    ground          = 0.1
    sub1...subn           = 0.2..0.2n
    other = altura+0.01
    furniture       = 0.3
    objects         = 0.3
    hero            = 0.4
    npc = dinámico con hero = 0.35 a 0.45
    top             = 0.6
    colisiones visibles = 0.8
    diálogo         = 0.8 a 1.0

#include "raylib.h"
#include "entidade.h"
#include <stdlib.h>

int main(void) {
    InitWindow(800, 600, "Atividade 8b - Reuso do modulo");
    SetTargetFPS(60);

    Entidade *paladino = entidadeCriar(ENTIDADE_JOGADOR, (Vector2){ 200, 300 });
    Entidade *goblin   = entidadeCriar(ENTIDADE_INIMIGO, (Vector2){ 400, 300 });
    Entidade *escudo    = entidadeCriar(ENTIDADE_ITEM,    (Vector2){ 600, 300 });

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            entidadeDesenhar(paladino);
            entidadeDesenhar(goblin);
            entidadeDesenhar(escudo);
        EndDrawing();
    }

    free(paladino);
    free(goblin);
    free(escudo);

    CloseWindow();
    return 0;
}

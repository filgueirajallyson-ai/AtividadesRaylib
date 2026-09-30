/*
 * Manipulação de Arquivos (texto e binário) com raylib
 * ---------------------------------------------------------------
 * Evolução da atividade5, com persistência em disco:
 *   1) "placar.txt": histórico de texto, agora com NOME + pontuação
 *      por linha (Exercício 1).
 *   2) "save.bin": estado completo do jogo em binário; agora pode
 *      ser apagado com a tecla DELETE (Exercício 2).
 *
 * Compilar (Linux, com raylib instalada):
 *   gcc atividade6.c -o atividade6 -lraylib -lm -lpthread -ldl -lrt -lX11
 */

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define LARGURA_JANELA  800
#define ALTURA_JANELA   600
#define RAIO_JOGADOR    20.0f
#define MAX_ENTIDADES   30
#define TOTAL_INIMIGOS  5
#define TOTAL_ITENS     6
#define TAM_NOME        16
#define ARQUIVO_PLACAR  "placar.txt"
#define ARQUIVO_SAVE    "save.bin"

typedef enum {
    ENTIDADE_JOGADOR,
    ENTIDADE_INIMIGO,
    ENTIDADE_ITEM
} TipoEntidade;

typedef union {
    int dano;
    int valor;
} ExtraEntidade;

typedef struct {
    TipoEntidade  tipo;
    Vector2       pos;
    float         raio;
    int           vida;
    Color         cor;
    ExtraEntidade extra;
} Entidade;

Entidade *vetorEntidades[MAX_ENTIDADES];
int totalEntidades = 0;

Entidade *criarEntidade(TipoEntidade tipo, Vector2 pos) {
    Entidade *e = (Entidade *)malloc(sizeof(Entidade));
    if (e == NULL) return NULL;

    e->tipo  = tipo;
    e->pos   = pos;
    e->raio  = (tipo == ENTIDADE_JOGADOR) ? RAIO_JOGADOR
             : (tipo == ENTIDADE_INIMIGO) ? 15.0f : 8.0f;

    switch (tipo) {
        case ENTIDADE_JOGADOR:
            e->vida = 100;
            e->cor  = BLUE;
            break;
        case ENTIDADE_INIMIGO:
            e->vida       = 40;
            e->cor        = MAROON;
            e->extra.dano = GetRandomValue(5, 15);
            break;
        case ENTIDADE_ITEM:
            e->vida        = 1;
            e->cor         = GOLD;
            e->extra.valor = GetRandomValue(5, 20);
            break;
    }
    return e;
}

void adicionarEntidade(Entidade *e) {
    if (e == NULL || totalEntidades >= MAX_ENTIDADES) return;
    vetorEntidades[totalEntidades] = e;
    totalEntidades++;
}

void removerEntidade(int indice) {
    if (indice < 0 || indice >= totalEntidades) return;
    free(vetorEntidades[indice]);
    vetorEntidades[indice] = vetorEntidades[totalEntidades - 1];
    totalEntidades--;
}

void liberarTodasEntidades(void) {
    for (int i = 0; i < totalEntidades; i++) free(vetorEntidades[i]);
    totalEntidades = 0;
}

bool colidiu(Entidade *a, Entidade *b) {
    float dx = a->pos.x - b->pos.x;
    float dy = a->pos.y - b->pos.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    return distancia <= (a->raio + b->raio);
}

void desenharEntidade(Entidade *e) {
    DrawCircleV(e->pos, e->raio, e->cor);
    if (e->tipo == ENTIDADE_INIMIGO) {
        DrawText(TextFormat("%d", e->vida), e->pos.x - 8, e->pos.y - 26, 14, BLACK);
    }
}

/* ---- arquivo de TEXTO: histórico "nome pontuacao" (fprintf/fscanf) ---- */
void salvarPlacarTexto(const char *nome, int pontuacao) {
    FILE *arquivo = fopen(ARQUIVO_PLACAR, "a"); // "a": anexa ao final, modo texto
    if (arquivo == NULL) return;

    fprintf(arquivo, "%s %d\n", nome, pontuacao); // EXERCÍCIO 1: nome + pontuação
    fclose(arquivo);
}

/* lê todos os pares "nome valor" e devolve a maior pontuação encontrada */
int lerMelhorPontuacao(void) {
    FILE *arquivo = fopen(ARQUIVO_PLACAR, "r"); // "r": leitura, modo texto
    if (arquivo == NULL) return 0;

    int melhor = 0, valor = 0;
    char nomeLido[TAM_NOME];
    // %15s lê no máximo 15 caracteres (+ '\0'), evitando estouro do buffer
    while (fscanf(arquivo, "%15s %d", nomeLido, &valor) == 2) {
        if (valor > melhor) melhor = valor;
    }
    fclose(arquivo);
    return melhor;
}

/* ---- arquivo BINÁRIO: estado completo do jogo (fwrite/fread) ---- */
bool salvarJogoBinario(void) {
    FILE *arquivo = fopen(ARQUIVO_SAVE, "wb");
    if (arquivo == NULL) return false;

    fwrite(&totalEntidades, sizeof(int), 1, arquivo);
    for (int i = 0; i < totalEntidades; i++) {
        fwrite(vetorEntidades[i], sizeof(Entidade), 1, arquivo);
    }

    fclose(arquivo);
    return true;
}

bool carregarJogoBinario(void) {
    FILE *arquivo = fopen(ARQUIVO_SAVE, "rb");
    if (arquivo == NULL) return false;

    int totalSalvo = 0;
    if (fread(&totalSalvo, sizeof(int), 1, arquivo) != 1) {
        fclose(arquivo);
        return false;
    }

    liberarTodasEntidades();
    for (int i = 0; i < totalSalvo; i++) {
        Entidade *e = (Entidade *)malloc(sizeof(Entidade));
        if (fread(e, sizeof(Entidade), 1, arquivo) != 1) {
            free(e);
            break;
        }
        adicionarEntidade(e);
    }

    fclose(arquivo);
    return true;
}

int main(void) {
    srand((unsigned int)time(NULL));

    InitWindow(LARGURA_JANELA, ALTURA_JANELA, "Atividade 6 - Manipulacao de Arquivos (texto e binario)");
    SetTargetFPS(60);

    /* ---- EXERCÍCIO 1: tela para digitar o nome do jogador ---- */
    char nomeJogador[TAM_NOME] = "";
    int tamNome = 0;
    bool nomeConfirmado = false;

    while (!WindowShouldClose() && !nomeConfirmado) {
        int c = GetCharPressed();
        while (c > 0) {
            // só caracteres visíveis (sem espaço, para o fscanf ler como uma palavra só)
            if (c > 32 && c < 127 && tamNome < TAM_NOME - 1) {
                nomeJogador[tamNome++] = (char)c;
                nomeJogador[tamNome] = '\0';
            }
            c = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && tamNome > 0) {
            nomeJogador[--tamNome] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER) && tamNome > 0) {
            nomeConfirmado = true;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Digite seu nome e pressione ENTER:", 200, 230, 24, DARKGRAY);
            DrawRectangle(200, 270, 400, 40, LIGHTGRAY);
            DrawText(nomeJogador, 210, 278, 24, BLACK);
            DrawText("(ate 15 caracteres, sem espacos)", 200, 320, 16, GRAY);
        EndDrawing();
    }

    if (!nomeConfirmado) { // janela fechada durante a digitação
        CloseWindow();
        return 0;
    }

    /* ---- jogo ---- */
    Entidade *jogador = criarEntidade(ENTIDADE_JOGADOR,
                                      (Vector2){ LARGURA_JANELA / 2.0f, ALTURA_JANELA / 2.0f });
    adicionarEntidade(jogador);

    for (int i = 0; i < TOTAL_INIMIGOS; i++) {
        Vector2 pos = { GetRandomValue(30, LARGURA_JANELA - 30), GetRandomValue(30, ALTURA_JANELA - 30) };
        adicionarEntidade(criarEntidade(ENTIDADE_INIMIGO, pos));
    }
    for (int i = 0; i < TOTAL_ITENS; i++) {
        Vector2 pos = { GetRandomValue(30, LARGURA_JANELA - 30), GetRandomValue(30, ALTURA_JANELA - 30) };
        adicionarEntidade(criarEntidade(ENTIDADE_ITEM, pos));
    }

    int pontuacao = 0;
    int melhorPontuacao = lerMelhorPontuacao();
    char mensagem[64] = "";
    float tempoMensagem = 0.0f;

    while (!WindowShouldClose()) {

        float vel = 250.0f * GetFrameTime();
        if (IsKeyDown(KEY_RIGHT)) jogador->pos.x += vel;
        if (IsKeyDown(KEY_LEFT))  jogador->pos.x -= vel;
        if (IsKeyDown(KEY_UP))    jogador->pos.y -= vel;
        if (IsKeyDown(KEY_DOWN))  jogador->pos.y += vel;

        for (int i = 1; i < totalEntidades; i++) {
            Entidade *e = vetorEntidades[i];
            if (!colidiu(jogador, e)) continue;

            if (e->tipo == ENTIDADE_ITEM) {
                pontuacao += e->extra.valor;
                removerEntidade(i);
                i--;
            } else if (e->tipo == ENTIDADE_INIMIGO) {
                jogador->vida -= e->extra.dano;
                if (jogador->vida < 0) jogador->vida = 0;
            }
        }

        if (IsKeyPressed(KEY_F5)) { // salva nome + pontuação no arquivo de texto
            salvarPlacarTexto(nomeJogador, pontuacao);
            if (pontuacao > melhorPontuacao) melhorPontuacao = pontuacao;
            TextCopy(mensagem, "Placar salvo em placar.txt!");
            tempoMensagem = 2.0f;
        }

        if (IsKeyPressed(KEY_F6)) {
            bool ok = salvarJogoBinario();
            TextCopy(mensagem, ok ? "Jogo salvo em save.bin!" : "Erro ao salvar save.bin!");
            tempoMensagem = 2.0f;
        }

        if (IsKeyPressed(KEY_F9)) {
            bool ok = carregarJogoBinario();
            if (ok) jogador = vetorEntidades[0];
            TextCopy(mensagem, ok ? "Jogo carregado de save.bin!" : "Nenhum save.bin encontrado!");
            tempoMensagem = 2.0f;
        }

        /* ---- EXERCÍCIO 2: apagar o save.bin ---- */
        if (IsKeyPressed(KEY_DELETE)) {
            if (remove(ARQUIVO_SAVE) == 0) { // 0 = sucesso
                TextCopy(mensagem, "save.bin apagado!");
            } else {
                TextCopy(mensagem, "Nenhum save encontrado");
            }
            tempoMensagem = 2.0f;
        }

        if (tempoMensagem > 0.0f) tempoMensagem -= GetFrameTime();

        BeginDrawing();
            ClearBackground(RAYWHITE);

            for (int i = 0; i < totalEntidades; i++) {
                desenharEntidade(vetorEntidades[i]);
            }

            DrawText(TextFormat("%s | Vida: %d   Pontuacao: %d   Recorde: %d",
                                 nomeJogador, jogador->vida, pontuacao, melhorPontuacao),
                     10, 10, 22, DARKGRAY);
            DrawText("F5 salva placar (texto) | F6 salva jogo (binario) | F9 carrega jogo (binario)",
                      10, 34, 18, GRAY);
            DrawText("DEL apaga o save.bin", 10, 56, 18, GRAY);
            DrawText("Setas movem o jogador | ESC sai", 10, ALTURA_JANELA - 25, 16, GRAY);

            if (tempoMensagem > 0.0f) {
                DrawText(mensagem, 10, 82, 20, DARKGREEN);
            }

        EndDrawing();
    }

    liberarTodasEntidades();

    CloseWindow();
    return 0;
}

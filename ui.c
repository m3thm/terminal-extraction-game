#include "ui.h"
#include <stdio.h>
#include "input.h"  
#include "map.h"     


static void printBorder(int N)
{
    putchar('+');
    for (int c = 0; c < N; c++) {
        fputs("---+", stdout);
    }
    putchar('\n');
}

static char playerSymbolAt(const GameState *game, int r, int c)
{
    for (int i = 0; i < game->numPlayers; i++) {
        const Player *player = &game->players[i];
        if (player->active && player->row == r && player->col == c) {
            return player->symbol;
        }
    }
    return '\0';
}

static char renderCharAt(const GameState *game, int r, int c)
{
    char ps = playerSymbolAt(game, r, c);
    if (ps != '\0') return ps;

    CellType t;
    if (!getCellType(&game->map, r, c, &t)) return '?';
    return cellToChar(t);
}

void renderGame(const GameState *game)
{
    if (!game) return;

    clear_screen();

    int N = game->map.N;

    /* ---- HUD ---- */
    int cp = game->currentPlayer;
    if (cp < 0 || cp >= game->numPlayers) cp = 0;
    const Player *p = &game->players[cp];

    printf("Turn: %d | Current: %c | Lives: %d | Intel: %d/%d\n",
           game->turnNumber,
           p->symbol,
           p->lives,
           p->intel,
           game->requiredIntel);

    puts("Controls: W A S D = move | Q = quit");
    puts("Goal: Collect all Intel (I), then reach Extraction (X)\n");

    /* Optional coordinate header (looks “gamey” and helps debugging) */
    printf("    ");
    for (int c = 0; c < N; c++) {
        printf("%2d  ", c);
    }
    putchar('\n');

    /* ---- Board ---- */
    for (int r = 0; r < N; r++) {
        printBorder(N);

        /* Row label */
        printf("%2d  ", r);

        for (int c = 0; c < N; c++) {
            char ch = renderCharAt(game, r, c);
            printf("| %c ", ch);
        }
        puts("|");
    }
    printBorder(N);
}

void renderEndScreen(const GameState *game)
{
    if (!game) return;

    clear_screen();

    if (game->winnerIndex >= 0 && game->winnerIndex < game->numPlayers) {
        printf("GAME OVER — Winner: %c\n", game->players[game->winnerIndex].symbol);
    } else {
        /* Phase 1 (single player): fallback message */
        const Player *p = &game->players[0];
        if (!p->active) {
            puts("GAME OVER — You are inactive.");
        } else {
            puts("GAME OVER");
        }
    }
}

